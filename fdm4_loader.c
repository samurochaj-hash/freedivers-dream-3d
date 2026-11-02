#include "fdm4_loader.h"

typedef struct { const uint8_t *p; uint32_t size, offset; } Reader;

static int read_bytes(Reader *r, void *out, uint32_t n) {
    uint32_t i;
    if (!r || n > r->size - r->offset) return 0;
    for (i=0;i<n;i++) ((uint8_t*)out)[i]=r->p[r->offset+i];
    r->offset += n;
    return 1;
}
static int read_u16(Reader *r,uint16_t *v) {
    uint8_t b[2]; if(!read_bytes(r,b,2))return 0;
    *v=(uint16_t)(b[0] | ((uint16_t)b[1]<<8)); return 1;
}
static int read_i16(Reader *r,int16_t *v) {
    uint16_t u; if(!read_u16(r,&u))return 0; *v=(int16_t)u; return 1;
}
static int read_u32(Reader *r,uint32_t *v) {
    uint8_t b[4]; if(!read_bytes(r,b,4))return 0;
    *v=(uint32_t)b[0] | ((uint32_t)b[1]<<8) |
       ((uint32_t)b[2]<<16) | ((uint32_t)b[3]<<24); return 1;
}
static int read_f32(Reader *r,float *v) {
    uint32_t bits; if(!read_u32(r,&bits))return 0;
    /* PS2 and common desktop targets use IEEE-754 binary32. */
    { union { uint32_t u; float f; } cv; cv.u=bits; *v=cv.f; }
    return 1;
}
static int read_vec3(Reader *r,AnimVec3 *v) {
    return read_f32(r,&v->x)&&read_f32(r,&v->y)&&read_f32(r,&v->z);
}
static int read_quat(Reader *r,AnimQuat *q) {
    return read_f32(r,&q->x)&&read_f32(r,&q->y)&&
           read_f32(r,&q->z)&&read_f32(r,&q->w);
}
static int read_transform(Reader *r,AnimTransform *t) {
    return read_vec3(r,&t->position)&&read_quat(r,&t->rotation)&&
           read_vec3(r,&t->scale);
}
static int read_matrix(Reader *r,SkinMatrix4 *m) {
    int i; for(i=0;i<16;i++) if(!read_f32(r,&m->m[i]))return 0; return 1;
}
static int finite_float(float f) {
    /* Reject NaN and infinities without requiring libm. */
    return (f==f && f<=3.4028230e+38f && f>=-3.4028230e+38f);
}
static int valid_transform(const AnimTransform *t) {
    return finite_float(t->position.x)&&finite_float(t->position.y)&&finite_float(t->position.z)&&
           finite_float(t->rotation.x)&&finite_float(t->rotation.y)&&
           finite_float(t->rotation.z)&&finite_float(t->rotation.w)&&
           finite_float(t->scale.x)&&finite_float(t->scale.y)&&finite_float(t->scale.z);
}

FDM4Result fdm4_load_memory(const uint8_t *data,uint32_t data_size,
                            FDM4Buffers *b,FDM4Counts *out) {
    Reader r;
    uint32_t magic,vc,ic,kc;
    uint16_t version,jc,cc,tc,reserved;
    uint32_t i;
    if(!data||!b||!out)return FDM4_ERR_ARGUMENT;
    r.p=data;r.size=data_size;r.offset=0;
    if(!read_u32(&r,&magic)||!read_u16(&r,&version)||
       !read_u16(&r,&reserved)||!read_u32(&r,&vc)||
       !read_u32(&r,&ic)||!read_u16(&r,&jc)||
       !read_u16(&r,&cc)||!read_u16(&r,&tc)||!read_u16(&r,&reserved)||
       !read_u32(&r,&kc)) return FDM4_ERR_TRUNCATED;
    if(magic!=FDM4_MAGIC)return FDM4_ERR_HEADER;
    if(version!=FDM4_VERSION)return FDM4_ERR_VERSION;
    if(vc==0||vc>FDM4_MAX_VERTICES||ic>FDM4_MAX_INDICES||
       jc==0||jc>FDM4_MAX_JOINTS||cc>FDM4_MAX_CLIPS||
       tc>FDM4_MAX_CLIPS*FDM4_MAX_JOINTS||
       kc>FDM4_MAX_CLIPS*FDM4_MAX_JOINTS*FDM4_MAX_KEYS_PER_TRACK)
        return FDM4_ERR_LIMIT;
    if(!b->vertices||!b->weights||!b->indices||!b->joints||
       !b->inverse_bind||(tc&&!b->tracks)||(kc&&!b->keys)||(cc&&!b->clips))
        return FDM4_ERR_BUFFER;

    for(i=0;i<vc;i++) {
        if(!read_vec3(&r,&b->vertices[i].position)||
           !read_vec3(&r,&b->vertices[i].normal))return FDM4_ERR_TRUNCATED;
        if(!finite_float(b->vertices[i].position.x)||!finite_float(b->vertices[i].position.y)||
           !finite_float(b->vertices[i].position.z)||!finite_float(b->vertices[i].normal.x)||
           !finite_float(b->vertices[i].normal.y)||!finite_float(b->vertices[i].normal.z))
            return FDM4_ERR_DATA;
        {
            int k; float sum=0.0f;
            for(k=0;k<4;k++) {
                uint16_t joint; float weight;
                if(!read_u16(&r,&joint)||!read_f32(&r,&weight))return FDM4_ERR_TRUNCATED;
                if(!finite_float(weight)||weight<0.0f)return FDM4_ERR_DATA;
                if(weight>0.0f && joint>=jc)return FDM4_ERR_DATA;
                b->weights[i].joint[k]=joint;b->weights[i].weight[k]=weight;sum+=weight;
            }
            if(sum>0.000001f) {
                for(k=0;k<4;k++)b->weights[i].weight[k]/=sum;
            }
        }
    }
    for(i=0;i<ic;i++) {
        if(!read_u32(&r,&b->indices[i]))return FDM4_ERR_TRUNCATED;
        if(b->indices[i]>=vc)return FDM4_ERR_DATA;
    }
    for(i=0;i<jc;i++) {
        if(!read_i16(&r,&b->joints[i].parent_index)||
           !read_u16(&r,&b->joints[i].flags)||
           !read_transform(&r,&b->joints[i].bind_local)||
           !read_matrix(&r,&b->inverse_bind[i]))return FDM4_ERR_TRUNCATED;
        if(b->joints[i].parent_index < -1 ||
           b->joints[i].parent_index >= (int16_t)i ||
           !valid_transform(&b->joints[i].bind_local))return FDM4_ERR_DATA;
    }
    for(i=0;i<tc;i++) {
        if(!read_u16(&r,&b->tracks[i].joint_index)||
           !read_u16(&r,&b->tracks[i].key_count))return FDM4_ERR_TRUNCATED;
        if(b->tracks[i].joint_index>=jc||b->tracks[i].key_count==0||
           b->tracks[i].key_count>FDM4_MAX_KEYS_PER_TRACK)return FDM4_ERR_DATA;
        /* Keyframe blocks are stored sequentially; track keys point into shared buffer. */
        b->tracks[i].keys = &b->keys[0]; /* reassigned after key data is read below */
    }
    {
        uint32_t key_cursor=0;
        for(i=0;i<tc;i++) {
            uint16_t j; /* track index already parsed; key counts preserved */
            uint16_t count=b->tracks[i].key_count;
            b->tracks[i].keys=&b->keys[key_cursor];
            for(j=0;j<count;j++) {
                AnimKeyframe *key=&b->keys[key_cursor++];
                if(!read_f32(&r,&key->time_seconds)||
                   !read_transform(&r,&key->transform))return FDM4_ERR_TRUNCATED;
                if(!finite_float(key->time_seconds)||key->time_seconds<0.0f||
                   !valid_transform(&key->transform))return FDM4_ERR_DATA;
                if(j>0 && key->time_seconds<=b->keys[key_cursor-2].time_seconds)
                    return FDM4_ERR_DATA;
            }
        }
        if(key_cursor!=kc)return FDM4_ERR_DATA;
    }
    for(i=0;i<cc;i++) {
        uint8_t name[ANIM_MAX_NAME];
        uint16_t first_track,track_count;
        uint32_t j;
        uint8_t clip_flags;
        if(!read_bytes(&r,name,ANIM_MAX_NAME)||
           !read_f32(&r,&b->clips[i].duration_seconds)||
           !read_u16(&r,&b->clips[i].joint_count)||
           !read_u16(&r,&track_count)||
           !read_u16(&r,&first_track)||
           !read_bytes(&r,&clip_flags,1)||
           !read_bytes(&r,&b->clips[i].looping,1))return FDM4_ERR_TRUNCATED;
        if(!finite_float(b->clips[i].duration_seconds)||
           b->clips[i].duration_seconds<=0||first_track>tc||
           track_count>tc-first_track||b->clips[i].joint_count>jc)
            return FDM4_ERR_DATA;
        for(j=0;j<ANIM_MAX_NAME;j++)b->clips[i].name[j]=(char)name[j];
        b->clips[i].tracks=&b->tracks[first_track];
        b->clips[i].track_count=track_count;
        if(b->clips[i].looping>1 || clip_flags!=0)return FDM4_ERR_DATA;
    }
    if(r.offset!=r.size)return FDM4_ERR_DATA;
    out->vertex_count=vc;out->index_count=ic;out->joint_count=jc;
    out->clip_count=cc;out->track_count=tc;out->key_count=kc;
    return FDM4_OK;
}
