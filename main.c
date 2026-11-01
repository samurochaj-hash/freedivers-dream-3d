/*
 * Freediver's Dream 3D - original homebrew prototype for PlayStation 2.
 *
 * Dive down a guide rope, tag the plate, swim back up before you black out.
 * Everything is procedural (no external assets): a tiny CPU 3D pipeline
 * (camera, near-plane clipping, fog) drawing lines and sprites through gsKit.
 *
 * Controls (DualShock 2):
 *   Left stick / D-pad : steer (up = nose down / dive, down = nose up)
 *   X                  : kick (speed, but burns air)
 *   R1                 : equalize ears (releases pressure)
 *   START              : start / restart
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>
#include <gsKit.h>
#include <dmaKit.h>

/* ------------------------------------------------------------------ */
/* small math helpers                                                  */
/* ------------------------------------------------------------------ */
typedef struct { float x, y, z; } V3;

static V3 v3(float x, float y, float z) { V3 r; r.x = x; r.y = y; r.z = z; return r; }
static V3 vadd(V3 a, V3 b) { return v3(a.x + b.x, a.y + b.y, a.z + b.z); }
static V3 vsub(V3 a, V3 b) { return v3(a.x - b.x, a.y - b.y, a.z - b.z); }
static V3 vmul(V3 a, float s) { return v3(a.x * s, a.y * s, a.z * s); }
static float vdot(V3 a, V3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static float vlen(V3 a) { return sqrtf(vdot(a, a)); }

static float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
static float lerpf(float a, float b, float t) { return a + (b - a) * t; }

static uint32_t rng_state = 12345u;
static float frand(void)
{
    rng_state = rng_state * 1664525u + 1013904223u;
    return (float)(rng_state >> 8) * (1.0f / 16777216.0f);
}
static float frange(float a, float b) { return a + (b - a) * frand(); }

static void basis(float yaw, float pitch, V3 *f, V3 *r, V3 *u)
{
    float sy = sinf(yaw), cy = cosf(yaw), sp = sinf(pitch), cp = cosf(pitch);
    *f = v3(sy * cp, sp, cy * cp);
    *r = v3(cy, 0.0f, -sy);
    *u = v3(-sy * sp, cp, -cy * sp);
}

/* ------------------------------------------------------------------ */
/* graphics                                                            */
/* ------------------------------------------------------------------ */
static GSGLOBAL *gs;
static float W = 640.0f, H = 448.0f, FOC = 480.0f, UI = 1.0f;
static float g_dim = 1.0f;

static int   env_wr, env_wg, env_wb;   /* water colour at camera depth */
static float env_amb = 1.0f;           /* ambient light at camera depth */

static V3 cam_pos, cam_f, cam_r, cam_u;
static float cam_yaw = 0.0f, cam_pitch = 0.0f;

static int U(float v) { return (int)(v * UI + 0.5f); }

static u64 rgb(int r, int g, int b)
{
    r = (int)(clampf((float)r, 0, 255) * g_dim);
    g = (int)(clampf((float)g, 0, 255) * g_dim);
    b = (int)(clampf((float)b, 0, 255) * g_dim);
    return GS_SETREG_RGBAQ(r, g, b, 0x80, 0x00);
}

static void sprite(float x1, float y1, float x2, float y2, int r, int g, int b)
{
    gsKit_prim_sprite(gs, x1, y1, x2, y2, 1, rgb(r, g, b));
}

static void line2d(float x1, float y1, float x2, float y2, int r, int g, int b)
{
    gsKit_prim_line(gs, x1, y1, x2, y2, 1, rgb(r, g, b));
}

static void water_color(float depth, int *r, int *g, int *b)
{
    float t = clampf(depth / 42.0f, 0.0f, 1.0f);
    *r = (int)lerpf(34.0f, 3.0f, t);
    *g = (int)lerpf(158.0f, 16.0f, t);
    *b = (int)lerpf(212.0f, 54.0f, t);
}

/* Liang-Barsky clip against the screen (keeps GS coordinates sane). */
static int clip_line(float *x0, float *y0, float *x1, float *y1)
{
    float xmin = -8.0f, ymin = -8.0f, xmax = W + 8.0f, ymax = H + 8.0f;
    float dx = *x1 - *x0, dy = *y1 - *y0, t0 = 0.0f, t1 = 1.0f;
    float p[4], q[4];
    int i;
    p[0] = -dx; p[1] = dx; p[2] = -dy; p[3] = dy;
    q[0] = *x0 - xmin; q[1] = xmax - *x0; q[2] = *y0 - ymin; q[3] = ymax - *y0;
    for (i = 0; i < 4; i++) {
        if (p[i] == 0.0f) {
            if (q[i] < 0.0f) return 0;
        } else {
            float r = q[i] / p[i];
            if (p[i] < 0.0f) { if (r > t1) return 0; if (r > t0) t0 = r; }
            else             { if (r < t0) return 0; if (r < t1) t1 = r; }
        }
    }
    {
        float nx0 = *x0 + t0 * dx, ny0 = *y0 + t0 * dy;
        float nx1 = *x0 + t1 * dx, ny1 = *y0 + t1 * dy;
        *x0 = nx0; *y0 = ny0; *x1 = nx1; *y1 = ny1;
    }
    return 1;
}

/* 3D line with near-plane clipping, depth fog and ambient dimming. */
static void line3(V3 a, V3 b, int r, int g, int bl, float fog, int thick)
{
    const float N = 0.25f;
    V3 d1 = vsub(a, cam_pos), d2 = vsub(b, cam_pos);
    float ax = vdot(d1, cam_r), ay = vdot(d1, cam_u), az = vdot(d1, cam_f);
    float bx = vdot(d2, cam_r), by = vdot(d2, cam_u), bz = vdot(d2, cam_f);
    float zm, f, ka, kb, x0, y0, x1, y1;
    int rr, gg, bb;

    if (az < N && bz < N) return;
    if (az < N) { float t = (N - az) / (bz - az); ax += (bx - ax) * t; ay += (by - ay) * t; az = N; }
    else if (bz < N) { float t = (N - bz) / (az - bz); bx += (ax - bx) * t; by += (ay - by) * t; bz = N; }

    zm = (az + bz) * 0.5f;
    f = zm / fog;
    if (f >= 1.0f) return;
    f = f * f;
    rr = (int)(r * env_amb * (1.0f - f) + env_wr * f);
    gg = (int)(g * env_amb * (1.0f - f) + env_wg * f);
    bb = (int)(bl * env_amb * (1.0f - f) + env_wb * f);

    ka = FOC / az; kb = FOC / bz;
    x0 = W * 0.5f + ax * ka; y0 = H * 0.5f - ay * ka;
    x1 = W * 0.5f + bx * kb; y1 = H * 0.5f - by * kb;
    if (!clip_line(&x0, &y0, &x1, &y1)) return;

    line2d(x0, y0, x1, y1, rr, gg, bb);
    if (thick) {
        line2d(x0 + 1.0f, y0, x1 + 1.0f, y1, rr, gg, bb);
        line2d(x0, y0 + 1.0f, x1, y1 + 1.0f, rr, gg, bb);
    }
}

static int proj(V3 p, float *sx, float *sy, float *z)
{
    V3 d = vsub(p, cam_pos);
    float vz = vdot(d, cam_f), k;
    if (vz < 0.3f) return 0;
    k = FOC / vz;
    *sx = W * 0.5f + vdot(d, cam_r) * k;
    *sy = H * 0.5f - vdot(d, cam_u) * k;
    *z = vz;
    return 1;
}

/* ------------------------------------------------------------------ */
/* tiny 3x5 vector font                                                */
/* ------------------------------------------------------------------ */
typedef struct { char c; uint8_t row[5]; } Glyph;
static const Glyph FONT[] = {
    {'0',{7,5,5,5,7}}, {'1',{2,6,2,2,7}}, {'2',{7,1,7,4,7}}, {'3',{7,1,7,1,7}},
    {'4',{5,5,7,1,1}}, {'5',{7,4,7,1,7}}, {'6',{7,4,7,5,7}}, {'7',{7,1,2,2,2}},
    {'8',{7,5,7,5,7}}, {'9',{7,5,7,1,7}},
    {'A',{2,5,7,5,5}}, {'B',{6,5,6,5,6}}, {'C',{3,4,4,4,3}}, {'D',{6,5,5,5,6}},
    {'E',{7,4,6,4,7}}, {'F',{7,4,6,4,4}}, {'G',{3,4,5,5,3}}, {'H',{5,5,7,5,5}},
    {'I',{7,2,2,2,7}}, {'J',{1,1,1,5,2}}, {'K',{5,5,6,5,5}}, {'L',{4,4,4,4,7}},
    {'M',{5,7,7,5,5}}, {'N',{6,5,5,5,5}}, {'O',{2,5,5,5,2}}, {'P',{6,5,6,4,4}},
    {'Q',{2,5,5,7,3}}, {'R',{6,5,6,5,5}}, {'S',{3,4,2,1,6}}, {'T',{7,2,2,2,2}},
    {'U',{5,5,5,5,7}}, {'V',{5,5,5,5,2}}, {'W',{5,5,7,7,5}}, {'X',{5,5,2,5,5}},
    {'Y',{5,5,2,2,2}}, {'Z',{7,1,2,4,7}},
    {'.',{0,0,0,0,2}}, {'-',{0,0,7,0,0}}, {':',{0,2,0,2,0}}, {'\'',{2,2,0,0,0}},
    {'/',{1,1,2,4,4}}, {'+',{0,2,7,2,0}}, {'!',{2,2,2,0,2}}, {' ',{0,0,0,0,0}},
};
#define NFONT ((int)(sizeof(FONT) / sizeof(FONT[0])))

static int text_scale(float s) { int v = (int)(s * UI + 0.5f); return v < 1 ? 1 : v; }
static int text_width(const char *s, int sc) { int n = (int)strlen(s); return n > 0 ? n * 4 * sc - sc : 0; }

static void text_pass(int x, int y, int sc, const char *s, int r, int g, int b)
{
    for (; *s; s++, x += 4 * sc) {
        int gi, row;
        for (gi = 0; gi < NFONT; gi++) if (FONT[gi].c == *s) break;
        if (gi == NFONT) continue;
        for (row = 0; row < 5; row++) {
            int bits = FONT[gi].row[row], col = 0;
            while (col < 3) {
                if (bits & (4 >> col)) {
                    int run = 1;
                    while (col + run < 3 && (bits & (4 >> (col + run)))) run++;
                    sprite((float)(x + col * sc), (float)(y + row * sc),
                           (float)(x + (col + run) * sc), (float)(y + (row + 1) * sc), r, g, b);
                    col += run;
                } else col++;
            }
        }
    }
}

/* text with a dark drop shadow so it stays readable over any water colour */
static void text(int x, int y, int sc, const char *s, int r, int g, int b)
{
    int off = sc >= 4 ? 2 : 1;
    text_pass(x + off, y + off, sc, s, 0, 14, 36);
    text_pass(x, y, sc, s, r, g, b);
}

static void text_center(int cy, int sc, const char *s, int r, int g, int b)
{
    text((int)(W * 0.5f) - text_width(s, sc) / 2, cy, sc, s, r, g, b);
}

/* ------------------------------------------------------------------ */
/* input                                                               */
/* ------------------------------------------------------------------ */
static char padBuf[256] __attribute__((aligned(64)));
static int pad_btn, pad_prev, pad_lx = 128, pad_ly = 128, pad_ok;

static void pad_init(void)
{
    int i, st;
    SifInitRpc(0);
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:PADMAN", 0, NULL);
    padInit(0);
    pad_ok = padPortOpen(0, 0, padBuf);
    if (!pad_ok) return;
    for (i = 0; i < 400000; i++) {
        st = padGetState(0, 0);
        if (st == PAD_STATE_STABLE || st == PAD_STATE_FINDCTP1) break;
    }
    padSetMainMode(0, 0, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);
}

static void pad_poll(void)
{
    struct padButtonStatus b;
    pad_prev = pad_btn;
    if (!pad_ok) return;
    if (padGetState(0, 0) != PAD_STATE_STABLE) return;
    if (padRead(0, 0, &b) != 0) {
        pad_btn = 0xffff ^ b.btns;
        pad_lx = b.ljoy_h;
        pad_ly = b.ljoy_v;
    }
}

static int pressed(int mask) { return (pad_btn & mask) && !(pad_prev & mask); }

/* ------------------------------------------------------------------ */
/* world                                                               */
/* ------------------------------------------------------------------ */
enum { ST_TITLE, ST_DIVE, ST_SURFACED, ST_BLACKOUT };

#define NSCHOOL 8
#define FISH_PER 5
#define NB 48
#define NK 16

typedef struct { float cx, cy, cz, R, sp, ph; int r, g, b; } School;
typedef struct { V3 p; float vy, life, size, sway; int on; } Bub;

static School sch[NSCHOOL];
static Bub bub[NB];
static float kx[NK], kz[NK], kph[NK];

static int   state = ST_TITLE;
static float t_game = 0.0f, state_t = 0.0f, fade_in = 0.0f, hint_t = 0.0f;
static V3    dpos;
static float yaw, pitch, speed, o2, press, eq_cd, kick_phase, kick_amt, bub_t;
static float flash, squeeze_t;
static int   level = 1, target = 10, tagged = 0, score = 0, best = 0;
static float floor_d = 24.0f;

static float seabed(float x, float z)
{
    return -floor_d + 2.2f * sinf(x * 0.17f) * cosf(z * 0.13f) + 1.5f * sinf(x * 0.05f + z * 0.07f);
}

static void spawn_bubble(V3 p, float size)
{
    int i;
    for (i = 0; i < NB; i++) {
        if (!bub[i].on) {
            bub[i].on = 1; bub[i].p = p; bub[i].vy = frange(0.8f, 1.6f);
            bub[i].life = frange(3.0f, 8.0f); bub[i].size = size; bub[i].sway = frange(0.0f, 6.28f);
            return;
        }
    }
}

static void start_level(void)
{
    int i;
    static const int pal[6][3] = {
        {200, 210, 220}, {255, 150, 40}, {240, 220, 70}, {120, 200, 255}, {255, 110, 120}, {170, 255, 170}
    };
    target = 10 + 5 * (level - 1);
    floor_d = (float)target + 14.0f;
    dpos = v3(0.0f, 0.0f, -7.0f);
    yaw = 0.0f; pitch = 0.0f; speed = 0.0f;
    o2 = 1.0f; press = 0.0f; eq_cd = 0.0f; tagged = 0; flash = 0.0f; squeeze_t = 0.0f;
    fade_in = 0.0f; hint_t = 0.0f; kick_phase = 0.0f; kick_amt = 0.0f; bub_t = 0.0f;
    cam_yaw = 0.0f; cam_pitch = 0.0f;
    memset(bub, 0, sizeof(bub));
    rng_state = 777u + (uint32_t)level * 131u;
    for (i = 0; i < NSCHOOL; i++) {
        sch[i].cx = frange(-8.0f, 8.0f);
        sch[i].cz = frange(-8.0f, 8.0f);
        sch[i].cy = -frange(4.0f, floor_d - 4.0f);
        sch[i].R = frange(5.0f, 14.0f);
        sch[i].sp = frange(0.15f, 0.34f) * ((i & 1) ? 1.0f : -1.0f);
        sch[i].ph = frange(0.0f, 6.28f);
        sch[i].r = pal[i % 6][0]; sch[i].g = pal[i % 6][1]; sch[i].b = pal[i % 6][2];
    }
    for (i = 0; i < NK; i++) {
        kx[i] = frange(-24.0f, 24.0f);
        kz[i] = frange(-24.0f, 24.0f);
        if (fabsf(kx[i]) < 3.0f && fabsf(kz[i]) < 3.0f) kx[i] += 6.0f;
        kph[i] = frange(0.0f, 6.28f);
    }
}

static void new_game(void)
{
    level = 1; score = 0;
    start_level();
    state = ST_DIVE;
    state_t = 0.0f;
}

static void update_camera(float dt)
{
    V3 f, r, u, p;
    float k = fminf(1.0f, 3.5f * dt), sb;
    cam_yaw += (yaw - cam_yaw) * k;
    cam_pitch += (pitch * 0.55f - cam_pitch) * k;
    basis(cam_yaw, cam_pitch, &f, &r, &u);
    p = vadd(vadd(dpos, vmul(f, -4.6f)), vmul(u, 0.9f));
    if (p.y > 1.2f) p.y = 1.2f;
    sb = seabed(p.x, p.z) + 0.5f;
    if (p.y < sb) p.y = sb;
    cam_pos = p; cam_f = f; cam_r = r; cam_u = u;
}

static void update(float dt)
{
    int i;
    t_game += dt;
    state_t += dt;

    for (i = 0; i < NB; i++) {
        if (!bub[i].on) continue;
        bub[i].p.y += bub[i].vy * dt;
        bub[i].p.x += sinf(t_game * 3.0f + bub[i].sway) * 0.25f * dt;
        bub[i].life -= dt;
        if (bub[i].p.y > 0.0f || bub[i].life < 0.0f) bub[i].on = 0;
    }
    if (flash > 0.0f) flash -= dt;

    if (state == ST_TITLE) {
        float a = t_game * 0.22f;
        cam_pos = v3(sinf(a) * 9.0f, -7.0f + sinf(t_game * 0.3f) * 1.5f, cosf(a) * 9.0f);
        cam_yaw = atan2f(-cam_pos.x, -cam_pos.z);
        cam_pitch = 0.08f;
        basis(cam_yaw, cam_pitch, &cam_f, &cam_r, &cam_u);
        if (pressed(PAD_START)) new_game();
        return;
    }

    if (state == ST_DIVE) {
        float lx = ((float)pad_lx - 128.0f) / 127.0f, ly = ((float)pad_ly - 128.0f) / 127.0f;
        float depth = -dpos.y, vmax, drift, drain, ndepth, dd;
        int kicking;
        V3 f, r, u, vel, plate;

        fade_in += dt;
        hint_t += dt;
        if (fabsf(lx) < 0.18f) lx = 0.0f;
        if (fabsf(ly) < 0.18f) ly = 0.0f;
        if (pad_btn & PAD_LEFT)  lx -= 1.0f;
        if (pad_btn & PAD_RIGHT) lx += 1.0f;
        if (pad_btn & PAD_UP)    ly -= 1.0f;
        if (pad_btn & PAD_DOWN)  ly += 1.0f;
        lx = clampf(lx, -1.0f, 1.0f); ly = clampf(ly, -1.0f, 1.0f);

        yaw += lx * 1.7f * dt;
        pitch = clampf(pitch + ly * 1.5f * dt, -1.45f, 1.45f);

        kicking = (pad_btn & PAD_CROSS) != 0;
        vmax = (press >= 1.0f) ? 0.5f : 1.8f;
        if (kicking) speed = fminf(vmax, speed + 2.8f * dt);
        else         speed *= expf(-0.5f * dt);
        if (speed > vmax) speed = vmax;
        kick_amt += ((kicking ? 1.0f : 0.15f) - kick_amt) * fminf(1.0f, 6.0f * dt);
        kick_phase += dt * (2.0f + 9.0f * kick_amt);

        basis(yaw, pitch, &f, &r, &u);
        drift = clampf((12.0f - depth) * 0.04f, -0.8f, 0.5f);   /* buoyancy: floats up near the surface, sinks when deep */
        vel = vadd(vmul(f, speed), v3(0.0f, drift, 0.0f));
        dpos = vadd(dpos, vmul(vel, dt));
        {
            float sb = seabed(dpos.x, dpos.z) + 0.6f;
            if (dpos.y < sb) { dpos.y = sb; if (speed > 0.3f) speed = 0.3f; }
        }
        if (dpos.y > 0.0f) dpos.y = 0.0f;

        ndepth = -dpos.y;
        dd = ndepth - depth;
        if (dd > 0.0f) press += dd * 0.2f / (1.0f + depth * 0.12f);   /* ears build pressure on the way down */

        if (eq_cd > 0.0f) eq_cd -= dt;
        if (pressed(PAD_R1) && eq_cd <= 0.0f) {
            if (press > 0.1f) press = fmaxf(0.0f, press - 0.6f);
            eq_cd = 0.8f;
            o2 -= 0.003f;
            for (i = 0; i < 3; i++) spawn_bubble(vadd(dpos, vmul(f, 0.9f)), 0.07f);
        }
        press = clampf(press, 0.0f, 1.0f);

        drain = 0.007f + ndepth * 0.00012f + (kicking ? 0.0065f : 0.0f);
        if (press >= 1.0f) { drain += 0.03f; squeeze_t += dt; }
        else squeeze_t = 0.0f;

        if (ndepth < 0.35f) {                       /* at the surface: breathe, relax */
            o2 = fminf(1.0f, o2 + 0.35f * dt);
            press = fmaxf(0.0f, press - 0.8f * dt);
            if (tagged) { state = ST_SURFACED; state_t = 0.0f; update_camera(dt); return; }
        } else {
            o2 -= drain * dt;
        }

        plate = v3(0.0f, -(float)target, 0.0f);
        if (!tagged && vlen(vsub(dpos, plate)) < 2.4f) { tagged = 1; flash = 1.0f; }

        bub_t -= dt;
        if (bub_t <= 0.0f && ndepth > 0.5f) {
            bub_t = kicking ? 0.22f : 0.9f;
            spawn_bubble(vadd(dpos, vmul(f, 0.9f)), 0.06f);
        }

        if (o2 <= 0.0f && ndepth >= 0.35f) {
            o2 = 0.0f;
            state = ST_BLACKOUT; state_t = 0.0f;
            if (score > best) best = score;
        }
        update_camera(dt);
        return;
    }

    if (state == ST_SURFACED) {
        o2 = fminf(1.0f, o2 + 0.4f * dt);
        speed *= expf(-1.5f * dt);
        update_camera(dt);
        if (state_t > 2.8f) {
            score += target;
            if (score > best) best = score;
            level++;
            start_level();
            state = ST_DIVE; state_t = 0.0f;
        }
        return;
    }

    if (state == ST_BLACKOUT) {
        dpos.y -= 0.5f * dt;
        {
            float sb = seabed(dpos.x, dpos.z) + 0.6f;
            if (dpos.y < sb) dpos.y = sb;
        }
        pitch *= expf(-1.0f * dt);
        update_camera(dt);
        if (state_t > 3.0f && pressed(PAD_START)) new_game();
        return;
    }
}

/* ------------------------------------------------------------------ */
/* rendering                                                           */
/* ------------------------------------------------------------------ */
static void draw_background(void)
{
    const int N = 28;
    int i;
    float camdepth = -cam_pos.y;
    float campitch = asinf(clampf(cam_f.y, -1.0f, 1.0f));
    for (i = 0; i < N; i++) {
        float y0 = H * (float)i / (float)N, y1 = H * (float)(i + 1) / (float)N + 1.0f;
        float ym = (y0 + y1) * 0.5f;
        float ang = atanf((H * 0.5f - ym) / FOC);
        float sample = camdepth - sinf(campitch + ang) * 14.0f;
        int r, g, b;
        if (sample < 0.0f) {                       /* looking above the waterline: sky */
            float t = clampf(-sample / 10.0f, 0.0f, 1.0f);
            r = (int)lerpf(205.0f, 90.0f, t); g = (int)lerpf(232.0f, 160.0f, t); b = (int)lerpf(250.0f, 230.0f, t);
        } else {
            water_color(sample, &r, &g, &b);
        }
        sprite(0.0f, y0, W, y1, r, g, b);
    }
}

static void draw_rays(void)
{
    int i;
    if (-cam_pos.y > 30.0f) return;
    for (i = 0; i < 7; i++) {
        float bx = cam_pos.x + ((float)i - 3.0f) * 7.0f + sinf(t_game * 0.4f + (float)i) * 1.5f;
        float bz = cam_pos.z + 10.0f + (float)(i % 3) * 7.0f;
        line3(v3(bx, 0.0f, bz), v3(bx + 7.0f, -34.0f, bz + 3.0f), 120, 200, 230, 34.0f, 0);
    }
}

static void draw_surface(void)
{
    const float S = 6.0f;
    const int N = 4;
    float cx = floorf(cam_pos.x / S) * S, cz = floorf(cam_pos.z / S) * S;
    int i, j;
    V3 p[9][9];
    if (-cam_pos.y > 40.0f) return;
    for (i = -N; i <= N; i++)
        for (j = -N; j <= N; j++) {
            float x = cx + (float)i * S, z = cz + (float)j * S;
            p[i + N][j + N] = v3(x, 0.15f * sinf(x * 0.6f + t_game * 1.2f) * cosf(z * 0.5f + t_game), z);
        }
    for (i = 0; i < 2 * N + 1; i++)
        for (j = 0; j < 2 * N + 1; j++) {
            if (i < 2 * N) line3(p[i][j], p[i + 1][j], 175, 230, 250, 42.0f, 0);
            if (j < 2 * N) line3(p[i][j], p[i][j + 1], 175, 230, 250, 42.0f, 0);
        }
}

static void draw_seabed(void)
{
    const float S = 5.0f;
    const int N = 5;
    float cx = floorf(cam_pos.x / S) * S, cz = floorf(cam_pos.z / S) * S;
    int i, j;
    V3 p[11][11];
    for (i = -N; i <= N; i++)
        for (j = -N; j <= N; j++) {
            float x = cx + (float)i * S, z = cz + (float)j * S;
            p[i + N][j + N] = v3(x, seabed(x, z), z);
        }
    for (i = 0; i < 2 * N + 1; i++)
        for (j = 0; j < 2 * N + 1; j++) {
            int par = (i + j) & 1;
            if (i < 2 * N) line3(p[i][j], p[i + 1][j], 165 - par * 25, 140 - par * 20, 86, 40.0f, 0);
            if (j < 2 * N) line3(p[i][j], p[i][j + 1], 165 - par * 25, 140 - par * 20, 86, 40.0f, 0);
        }
}

static void draw_kelp(void)
{
    int i, k;
    for (i = 0; i < NK; i++) {
        V3 prev = v3(kx[i], seabed(kx[i], kz[i]), kz[i]);
        float dx = kx[i] - cam_pos.x, dz = kz[i] - cam_pos.z;
        if (dx * dx + dz * dz > 38.0f * 38.0f) continue;
        for (k = 1; k <= 4; k++) {
            V3 cur = v3(kx[i] + sinf(t_game * 0.9f + kph[i] + (float)k * 0.7f) * 0.3f * (float)k,
                        prev.y + 1.3f, kz[i]);
            line3(prev, cur, 40, 150, 80, 36.0f, 1);
            prev = cur;
        }
    }
}

static void draw_rope_and_plate(void)
{
    int m;
    float pulse = 0.6f + 0.4f * sinf(t_game * 4.0f);
    float h = 0.85f;
    int cr = tagged ? 70 : (int)(255 * pulse), cg = tagged ? 235 : (int)(210 * pulse), cb = tagged ? 120 : 40;
    V3 pc = v3(0.0f, -(float)target, 0.0f);

    line3(v3(0.0f, 0.5f, 0.0f), v3(0.0f, -((float)target + 2.5f), 0.0f), 235, 235, 235, 44.0f, 1);
    /* surface buoy */
    line3(v3(-0.4f, 0.0f, 0.0f), v3(0.0f, 0.5f, 0.0f), 255, 120, 30, 60.0f, 1);
    line3(v3(0.4f, 0.0f, 0.0f), v3(0.0f, 0.5f, 0.0f), 255, 120, 30, 60.0f, 1);
    line3(v3(-0.4f, 0.0f, 0.0f), v3(0.4f, 0.0f, 0.0f), 255, 120, 30, 60.0f, 1);
    /* depth ticks every 5 m */
    for (m = 5; m <= target + 2; m += 5) {
        line3(v3(-0.45f, -(float)m, 0.0f), v3(0.45f, -(float)m, 0.0f), 255, 255, 255, 40.0f, 0);
    }
    /* weighted plate */
    line3(vadd(pc, v3(-h, -h, 0)), vadd(pc, v3(h, -h, 0)), cr, cg, cb, 60.0f, 1);
    line3(vadd(pc, v3(h, -h, 0)), vadd(pc, v3(h, h, 0)), cr, cg, cb, 60.0f, 1);
    line3(vadd(pc, v3(h, h, 0)), vadd(pc, v3(-h, h, 0)), cr, cg, cb, 60.0f, 1);
    line3(vadd(pc, v3(-h, h, 0)), vadd(pc, v3(-h, -h, 0)), cr, cg, cb, 60.0f, 1);
    line3(vadd(pc, v3(-h, -h, 0)), vadd(pc, v3(h, h, 0)), cr, cg, cb, 60.0f, 0);
    line3(vadd(pc, v3(h, -h, 0)), vadd(pc, v3(-h, h, 0)), cr, cg, cb, 60.0f, 0);
    line3(v3(0.0f, -((float)target + 2.5f), 0.0f), v3(0.0f, -((float)target + 2.5f) - 0.7f, 0.0f), 180, 180, 190, 40.0f, 1);
}

static void draw_fish(void)
{
    int s, i;
    for (s = 0; s < NSCHOOL; s++) {
        float sgn = sch[s].sp > 0.0f ? 1.0f : -1.0f;
        for (i = 0; i < FISH_PER; i++) {
            float a = sch[s].ph + t_game * sch[s].sp + (float)i * 0.22f * sgn;
            float R = sch[s].R + sinf((float)i * 2.1f) * 1.2f;
            V3 p = v3(sch[s].cx + cosf(a) * R,
                      sch[s].cy + sinf(t_game * 0.6f + (float)i) * 0.6f + (float)i * 0.15f,
                      sch[s].cz + sinf(a) * R);
            V3 d = v3(-sinf(a) * sgn, 0.0f, cosf(a) * sgn);
            V3 side = v3(d.z, 0.0f, -d.x);
            V3 rel = vsub(p, cam_pos);
            V3 nose, tail, top, bot, tend, f1, f2;
            float wig;
            int cr = sch[s].r, cg = sch[s].g, cb = sch[s].b;
            if (vdot(rel, rel) > 34.0f * 34.0f) continue;
            wig = sinf(t_game * 8.0f + (float)i * 1.3f + (float)s) * 0.1f;
            nose = vadd(p, vmul(d, 0.55f));
            tail = vsub(p, vmul(d, 0.45f));
            top = vadd(vadd(p, v3(0.0f, 0.2f, 0.0f)), vmul(d, 0.05f));
            bot = vadd(vsub(p, v3(0.0f, 0.18f, 0.0f)), vmul(d, 0.05f));
            tend = vadd(vsub(tail, vmul(d, 0.3f)), vmul(side, wig));
            f1 = vadd(tend, v3(0.0f, 0.22f, 0.0f));
            f2 = vsub(tend, v3(0.0f, 0.22f, 0.0f));
            line3(nose, top, cr, cg, cb, 32.0f, 0);
            line3(top, tail, cr, cg, cb, 32.0f, 0);
            line3(tail, bot, cr, cg, cb, 32.0f, 0);
            line3(bot, nose, cr, cg, cb, 32.0f, 0);
            line3(tail, f1, cr, cg, cb, 32.0f, 0);
            line3(tail, f2, cr, cg, cb, 32.0f, 0);
            line3(f1, f2, cr, cg, cb, 32.0f, 0);
        }
    }
}

static void draw_diver(void)
{
    V3 f, r, u, head, neck, hips, hands, shL, shR, knL, knR, ftL, ftR;
    float ph = kick_phase, amp = 0.12f + 0.2f * kick_amt;
    int sr = 255, sg = 225, sb = 70;      /* suit (hi-vis yellow) */
    int fr = 255, fg = 100, fb = 40;      /* fins */
    int k;

    basis(yaw, pitch, &f, &r, &u);
    head  = vadd(dpos, vmul(f, 0.95f));
    neck  = vadd(dpos, vmul(f, 0.6f));
    hips  = vsub(dpos, vmul(f, 0.5f));
    hands = vadd(dpos, vmul(f, 1.75f));
    shL = vadd(neck, vmul(r, 0.2f));
    shR = vsub(neck, vmul(r, 0.2f));
    knL = vadd(vadd(vsub(hips, vmul(f, 0.45f)), vmul(r, 0.1f)), vmul(u, sinf(ph) * amp));
    knR = vsub(vadd(vsub(hips, vmul(f, 0.45f)), vmul(u, sinf(ph + 3.14f) * amp)), vmul(r, 0.1f));
    ftL = vadd(vadd(vsub(hips, vmul(f, 0.9f)), vmul(r, 0.1f)), vmul(u, sinf(ph + 0.7f) * amp * 2.0f));
    ftR = vsub(vadd(vsub(hips, vmul(f, 0.9f)), vmul(u, sinf(ph + 3.84f) * amp * 2.0f)), vmul(r, 0.1f));

    for (k = -1; k <= 1; k++) {                       /* torso, 3 strands */
        V3 o = vmul(r, 0.1f * (float)k);
        line3(vadd(hips, o), vadd(neck, o), sr, sg, sb, 60.0f, 1);
    }
    line3(neck, head, 235, 200, 170, 60.0f, 1);
    line3(vsub(head, vmul(r, 0.14f)), vadd(head, vmul(f, 0.0f)), 60, 60, 70, 60.0f, 1);   /* mask strap */
    line3(vadd(head, vmul(r, 0.14f)), vadd(head, vmul(u, 0.0f)), 60, 60, 70, 60.0f, 1);
    line3(shL, vadd(hands, vmul(r, 0.08f)), sr, sg, sb, 60.0f, 1);   /* arms overhead */
    line3(shR, vsub(hands, vmul(r, 0.08f)), sr, sg, sb, 60.0f, 1);
    line3(shL, shR, sr, sg, sb, 60.0f, 1);
    line3(hips, knL, sr, sg, sb, 60.0f, 1);
    line3(knL, ftL, sr, sg, sb, 60.0f, 1);
    line3(hips, knR, sr, sg, sb, 60.0f, 1);
    line3(knR, ftR, sr, sg, sb, 60.0f, 1);
    /* long blades */
    line3(ftL, vadd(vsub(ftL, vmul(f, 0.55f)), vmul(r, 0.22f)), fr, fg, fb, 60.0f, 1);
    line3(ftL, vsub(vsub(ftL, vmul(f, 0.55f)), vmul(r, 0.02f)), fr, fg, fb, 60.0f, 1);
    line3(ftR, vsub(vsub(ftR, vmul(f, 0.55f)), vmul(r, 0.22f)), fr, fg, fb, 60.0f, 1);
    line3(ftR, vadd(vsub(ftR, vmul(f, 0.55f)), vmul(r, 0.02f)), fr, fg, fb, 60.0f, 1);
}

static void draw_bubbles(void)
{
    int i;
    for (i = 0; i < NB; i++) {
        float sx, sy, z, rad;
        float f;
        int cr, cg, cb;
        if (!bub[i].on) continue;
        if (!proj(bub[i].p, &sx, &sy, &z)) continue;
        if (sx < -20 || sx > W + 20 || sy < -20 || sy > H + 20) continue;
        f = clampf(z / 34.0f, 0.0f, 1.0f);
        cr = (int)lerpf(235.0f, (float)env_wr, f * f);
        cg = (int)lerpf(250.0f, (float)env_wg, f * f);
        cb = (int)lerpf(255.0f, (float)env_wb, f * f);
        rad = clampf(bub[i].size * FOC / z, 1.5f, 14.0f);
        if (rad < 3.0f) {
            sprite(sx - rad, sy - rad, sx + rad, sy + rad, cr, cg, cb);
        } else {
            line2d(sx - rad, sy, sx, sy - rad, cr, cg, cb);
            line2d(sx, sy - rad, sx + rad, sy, cr, cg, cb);
            line2d(sx + rad, sy, sx, sy + rad, cr, cg, cb);
            line2d(sx, sy + rad, sx - rad, sy, cr, cg, cb);
        }
    }
}

static void bar(int x, int y, int w, int h, float v, int r, int g, int b)
{
    sprite((float)(x - 2), (float)(y - 2), (float)(x + w + 2), (float)(y + h + 2), 8, 24, 48);
    sprite((float)x, (float)y, (float)(x + w), (float)(y + h), 20, 50, 80);
    if (v > 0.0f) sprite((float)x, (float)y, (float)x + (float)w * clampf(v, 0.0f, 1.0f), (float)(y + h), r, g, b);
}

static void draw_hud(void)
{
    char buf[48];
    int sc2 = text_scale(2), sc3 = text_scale(3);
    int mx = U(40), my = U(30);
    float depth = -dpos.y;
    int x, y, gx, gy0, gy1, m;
    float maxd = (float)target + 8.0f, ty, dy;

    /* air / ears (bottom-left) */
    x = mx; y = (int)H - my - U(46);
    text(x, y, sc2, "AIR", 255, 255, 255);
    bar(x + U(64), y, U(190), U(12), o2,
        o2 > 0.3f ? 60 : 240, o2 > 0.3f ? 220 : (o2 > 0.15f ? 180 : 60), o2 > 0.3f ? 110 : 40);
    text(x, y + U(24), sc2, "EAR", 255, 255, 255);
    bar(x + U(64), y + U(24), U(190), U(12), press,
        press < 0.7f ? 90 : 255, press < 0.7f ? 200 : (press < 0.9f ? 160 : 60), press < 0.7f ? 255 : 50);

    /* depth readout (top-right) */
    snprintf(buf, sizeof(buf), "DEPTH %dM", (int)(depth + 0.5f));
    text((int)W - mx - text_width(buf, sc3), my, sc3, buf, 255, 255, 255);
    snprintf(buf, sizeof(buf), "TARGET %dM", target);
    text((int)W - mx - text_width(buf, sc2), my + U(30), sc2, buf, tagged ? 110 : 255, tagged ? 255 : 220, tagged ? 150 : 90);

    /* level / score (top-left) */
    snprintf(buf, sizeof(buf), "LEVEL %d", level);
    text(mx, my, sc2, buf, 255, 255, 255);
    snprintf(buf, sizeof(buf), "SCORE %dM", score);
    text(mx, my + U(22), sc2, buf, 200, 230, 255);

    /* vertical depth gauge (right) */
    gx = (int)W - mx - U(6); gy0 = my + U(70); gy1 = (int)H - my - U(60);
    sprite((float)(gx - 3), (float)(gy0 - 3), (float)(gx + 5), (float)(gy1 + 3), 8, 24, 48);
    sprite((float)gx, (float)gy0, (float)(gx + 2), (float)gy1, 40, 90, 130);
    for (m = 0; m <= (int)maxd; m += 5) {
        float yy = (float)gy0 + (float)(gy1 - gy0) * (float)m / maxd;
        sprite((float)(gx - 3), yy, (float)(gx + 5), yy + 1.0f, 120, 170, 200);
    }
    ty = (float)gy0 + (float)(gy1 - gy0) * (float)target / maxd;
    sprite((float)(gx - U(12)), ty - 1.0f, (float)(gx + U(14)), ty + 2.0f, tagged ? 80 : 255, tagged ? 240 : 220, tagged ? 130 : 60);
    dy = (float)gy0 + (float)(gy1 - gy0) * clampf(depth / maxd, 0.0f, 1.0f);
    sprite((float)(gx - U(8)), dy - U(3), (float)(gx + U(10)), dy + U(3), 255, 255, 255);

    /* guidance */
    if (press > 0.75f && ((int)(t_game * 6.0f) & 1)) {
        text_center((int)H - my - U(86), sc3, "EQUALIZE!  R1", 255, 150, 80);
    } else if (o2 < 0.2f && depth > 1.0f && ((int)(t_game * 5.0f) & 1)) {
        text_center((int)H - my - U(86), sc3, "LOW AIR - SURFACE!", 255, 90, 90);
    } else if (hint_t < 9.0f && level == 1) {
        text_center((int)H - my - U(86), sc2, "L STICK STEER   X KICK   R1 EQUALIZE", 235, 245, 255);
    } else if (!tagged) {
        text_center((int)H - my - U(86), sc2, "DIVE TO THE PLATE", 235, 245, 255);
    } else {
        text_center((int)H - my - U(86), sc2, "TAGGED!  SWIM UP", 120, 255, 160);
    }

    if (flash > 0.0f) {                         /* tag flash frame */
        float t = flash * 14.0f;
        sprite(0, 0, W, t, 120, 255, 170);
        sprite(0, H - t, W, H, 120, 255, 170);
        sprite(0, 0, t, H, 120, 255, 170);
        sprite(W - t, 0, W, H, 120, 255, 170);
    }
    if (squeeze_t > 0.0f && ((int)(t_game * 8.0f) & 1)) {   /* ear squeeze warning */
        sprite(0, 0, W, 6, 255, 40, 40);
        sprite(0, H - 6, W, H, 255, 40, 40);
    }
}

static void draw_world(void)
{
    water_color(-cam_pos.y, &env_wr, &env_wg, &env_wb);
    env_amb = lerpf(1.0f, 0.45f, clampf(-cam_pos.y / 45.0f, 0.0f, 1.0f));
    draw_background();
    draw_rays();
    draw_seabed();
    draw_kelp();
    draw_surface();
    draw_rope_and_plate();
    draw_fish();
}

static void render(void)
{
    gsKit_clear(gs, GS_SETREG_RGBAQ(0, 0, 0, 0, 0));

    if (state == ST_BLACKOUT) g_dim = clampf(1.0f - state_t / 2.2f, 0.0f, 1.0f);
    else if (state == ST_TITLE) g_dim = 1.0f;
    else g_dim = clampf(fade_in, 0.0f, 1.0f);

    if (state == ST_BLACKOUT && g_dim <= 0.0f) {
        char buf[48];
        g_dim = 1.0f;
        sprite(0, 0, W, H, 0, 0, 0);
        if (state_t > 2.4f) {
            text_center((int)(H * 0.25f), text_scale(6), "BLACKOUT", 255, 70, 70);
            snprintf(buf, sizeof(buf), "SCORE %dM", score);
            text_center((int)(H * 0.48f), text_scale(3), buf, 255, 255, 255);
            snprintf(buf, sizeof(buf), "BEST %dM", best);
            text_center((int)(H * 0.58f), text_scale(3), buf, 200, 230, 255);
            if (state_t > 3.0f && ((int)(t_game * 2.0f) & 1))
                text_center((int)(H * 0.78f), text_scale(3), "PRESS START", 255, 255, 255);
        }
        return;
    }

    draw_world();
    if (state != ST_TITLE) {
        draw_diver();
        draw_bubbles();
        draw_hud();
        if (state == ST_SURFACED) {
            char buf[32];
            text_center((int)(H * 0.28f), text_scale(5), "SURFACE OK!", 120, 255, 170);
            snprintf(buf, sizeof(buf), "+%dM", target);
            text_center((int)(H * 0.28f) + U(60), text_scale(4), buf, 255, 255, 255);
        }
    } else {
        draw_bubbles();
        text_center((int)(H * 0.16f), text_scale(6), "FREEDIVER'S", 255, 255, 255);
        text_center((int)(H * 0.16f) + U(46), text_scale(6), "DREAM 3D", 160, 235, 255);
        if ((int)(t_game * 2.0f) & 1) text_center((int)(H * 0.70f), text_scale(3), "PRESS START", 255, 255, 255);
        text_center((int)(H * 0.82f), text_scale(2), "L STICK STEER   X KICK   R1 EQUALIZE", 220, 240, 255);
        if (best > 0) {
            char buf[32];
            snprintf(buf, sizeof(buf), "BEST %dM", best);
            text_center((int)(H * 0.90f), text_scale(2), buf, 200, 230, 255);
        }
    }
}

/* ------------------------------------------------------------------ */
/* setup + main loop                                                   */
/* ------------------------------------------------------------------ */
static void gfx_init(void)
{
    gs = gsKit_init_global();
    gs->ZBuffering = GS_SETTING_OFF;
    gs->DoubleBuffering = GS_SETTING_ON;
    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);
    gsKit_init_screen(gs);
    gsKit_mode_switch(gs, GS_ONESHOT);
    W = (float)gs->Width;
    H = (float)gs->Height;
    FOC = W * 0.75f;
    UI = H / 448.0f;
}

int main(void)
{
    float dt;
    pad_init();
    gfx_init();
    dt = (gs->Mode == GS_MODE_PAL) ? (1.0f / 50.0f) : (1.0f / 60.0f);
    start_level();
    state = ST_TITLE;
    for (;;) {
        pad_poll();
        update(dt);
        render();
        gsKit_queue_exec(gs);
        gsKit_sync_flip(gs);
    }
    return 0;
}
