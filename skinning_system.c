#include "skinning_system.h"
static SkinVec3 add(SkinVec3 a,SkinVec3 b){SkinVec3 r={a.x+b.x,a.y+b.y,a.z+b.z};return r;}
static SkinVec3 mul(SkinVec3 a,float s){SkinVec3 r={a.x*s,a.y*s,a.z*s};return r;}
static SkinVec3 norm(SkinVec3 v){float n=v.x*v.x+v.y*v.y+v.z*v.z;if(n<1e-6f){SkinVec3 d={0,1,0};return d;}float q=1.0f;int i;for(i=0;i<8;i++)q*=1.5f-0.5f*n*q*q;return mul(v,q);}
void skin_matrix_identity(SkinMatrix4 *o){int i;if(!o)return;for(i=0;i<16;i++)o->m[i]=0;o->m[0]=o->m[5]=o->m[10]=o->m[15]=1;}
SkinVec3 skin_transform_point(const SkinMatrix4 *m,SkinVec3 p){SkinVec3 r={0,0,0};if(!m)return r;r.x=m->m[0]*p.x+m->m[1]*p.y+m->m[2]*p.z+m->m[3];r.y=m->m[4]*p.x+m->m[5]*p.y+m->m[6]*p.z+m->m[7];r.z=m->m[8]*p.x+m->m[9]*p.y+m->m[10]*p.z+m->m[11];return r;}
SkinVec3 skin_transform_vector(const SkinMatrix4 *m,SkinVec3 v){SkinVec3 r={0,0,0};if(!m)return r;r.x=m->m[0]*v.x+m->m[1]*v.y+m->m[2]*v.z;r.y=m->m[4]*v.x+m->m[5]*v.y+m->m[6]*v.z;r.z=m->m[8]*v.x+m->m[9]*v.y+m->m[10]*v.z;return r;}
SkinResult skin_deform_vertices(const SkinSourceVertex *s,const SkinVertexWeights *w,uint32_t count,const SkinMatrix4 *p,uint16_t pc,SkinOutputVertex *d){
 uint32_t i;if(!s||!w||!p||!d)return SKIN_ERROR_NULL;if(!pc)return SKIN_ERROR_CAPACITY;
 for(i=0;i<count;i++){SkinVec3 pos={0,0,0},nrm={0,0,0};float total=0;int k;
  for(k=0;k<SKIN_MAX_INFLUENCES;k++){float a=w[i].weight[k];uint16_t j=w[i].joint[k];if(a<=0)continue;if(j>=pc)return SKIN_ERROR_JOINT_INDEX;pos=add(pos,mul(skin_transform_point(&p[j],s[i].position),a));nrm=add(nrm,mul(skin_transform_vector(&p[j],s[i].normal),a));total+=a;}
  if(total<=1e-6f){d[i].position=s[i].position;d[i].normal=norm(s[i].normal);}else{d[i].position=mul(pos,1.0f/total);d[i].normal=norm(mul(nrm,1.0f/total));}
 }return SKIN_OK;
}
