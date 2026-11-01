#include "animation_system.h"
static float clampf(float v,float lo,float hi){return v<lo?lo:(v>hi?hi:v);}
static float lerpf(float a,float b,float t){return a+(b-a)*t;}
static AnimVec3 vlerp(AnimVec3 a,AnimVec3 b,float t){
 AnimVec3 r={lerpf(a.x,b.x,t),lerpf(a.y,b.y,t),lerpf(a.z,b.z,t)}; return r;
}
static AnimQuat qnorm(AnimQuat q){
 float n=q.x*q.x+q.y*q.y+q.z*q.z+q.w*q.w;
 if(n<0.000001f){AnimQuat id={0,0,0,1};return id;}
 /* Newton-Raphson reciprocal square root; portable C. */
 {float inv=1.0f;int i;for(i=0;i<8;i++)inv*=1.5f-0.5f*n*inv*inv;
 q.x*=inv;q.y*=inv;q.z*=inv;q.w*=inv;} return q;
}
static AnimQuat qlerp(AnimQuat a,AnimQuat b,float t){
 AnimQuat r; float d=a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w;
 if(d<0){b.x=-b.x;b.y=-b.y;b.z=-b.z;b.w=-b.w;}
 r.x=lerpf(a.x,b.x,t);r.y=lerpf(a.y,b.y,t);
 r.z=lerpf(a.z,b.z,t);r.w=lerpf(a.w,b.w,t);return qnorm(r);
}
void anim_transform_identity(AnimTransform *t){
 if(!t)return;
 t->position.x=t->position.y=t->position.z=0;
 t->rotation.x=t->rotation.y=t->rotation.z=0;t->rotation.w=1;
 t->scale.x=t->scale.y=t->scale.z=1;
}
static AnimTransform tlerp(AnimTransform a,AnimTransform b,float t){
 AnimTransform r;r.position=vlerp(a.position,b.position,t);
 r.scale=vlerp(a.scale,b.scale,t);r.rotation=qlerp(a.rotation,b.rotation,t);return r;
}
static AnimTransform sample_track(const AnimJointTrack *tr,float time){
 uint16_t i; AnimTransform id;
 if(!tr||!tr->keys||!tr->key_count){anim_transform_identity(&id);return id;}
 if(tr->key_count==1||time<=tr->keys[0].time_seconds)return tr->keys[0].transform;
 for(i=0;i+1<tr->key_count;i++){
  const AnimKeyframe *a=&tr->keys[i],*b=&tr->keys[i+1];
  if(time<=b->time_seconds){
   float span=b->time_seconds-a->time_seconds;
   float t=span>0.00001f?(time-a->time_seconds)/span:0;
   return tlerp(a->transform,b->transform,clampf(t,0,1));
  }
 }
 return tr->keys[tr->key_count-1].transform;
}
int anim_player_init(AnimPlayer *p,const AnimJoint *s,uint16_t joints,
 const AnimClip *c,uint16_t clips,AnimTransform *pose,uint16_t cap){
 uint16_t i;if(!p||!s||!pose||!joints||joints>ANIM_MAX_JOINTS||
 cap<joints||clips>ANIM_MAX_CLIPS||(clips&&!c))return 0;
 p->skeleton=s;p->joint_count=joints;p->clips=c;p->clip_count=clips;
 p->local_pose=pose;p->time_seconds=0;p->playback_speed=1;
 p->active_clip=-1;p->playing=0;
 for(i=0;i<joints;i++)pose[i]=s[i].bind_local;return 1;
}
int anim_player_play(AnimPlayer *p,int16_t index,float speed){
 if(!p||index<0||index>=(int16_t)p->clip_count||
 p->clips[index].duration_seconds<=0)return 0;
 p->active_clip=index;p->time_seconds=0;
 p->playback_speed=speed==0?1:speed;p->playing=1;return 1;
}
void anim_player_stop(AnimPlayer *p){
 uint16_t i;if(!p)return;p->playing=0;p->active_clip=-1;p->time_seconds=0;
 for(i=0;i<p->joint_count;i++)p->local_pose[i]=p->skeleton[i].bind_local;
}
void anim_sample_clip(const AnimClip *clip,float time,AnimTransform *out,uint16_t count){
 uint16_t i;if(!clip||!out)return;
 for(i=0;i<count;i++)anim_transform_identity(&out[i]);
 for(i=0;i<clip->track_count;i++){
  const AnimJointTrack *tr=&clip->tracks[i];
  if(tr->joint_index<count)out[tr->joint_index]=sample_track(tr,time);
 }
}
void anim_player_update(AnimPlayer *p,float dt){
 const AnimClip *clip;float duration;
 if(!p||!p->playing||p->active_clip<0||
 p->active_clip>=(int16_t)p->clip_count||dt<=0)return;
 clip=&p->clips[p->active_clip];duration=clip->duration_seconds;
 p->time_seconds+=dt*p->playback_speed;
 if(clip->looping){
  while(p->time_seconds>=duration)p->time_seconds-=duration;
  while(p->time_seconds<0)p->time_seconds+=duration;
 }else{
  p->time_seconds=clampf(p->time_seconds,0,duration);
  if((p->playback_speed>=0&&p->time_seconds>=duration)||
     (p->playback_speed<0&&p->time_seconds<=0))p->playing=0;
 }
 anim_sample_clip(clip,p->time_seconds,p->local_pose,p->joint_count);
}
