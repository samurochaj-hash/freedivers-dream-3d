# Model pipeline — Freediver's Dream 3D PS2

This folder is reserved for original or authorized model assets.

## Suggested authoring pipeline
1. Create a low/mid-poly character in Blender.
2. Use a simple 8-joint skeleton matching `DiverJoint` in `include/model.h`.
3. Keep materials and texture atlases modest in size.
4. Export an interchange format (for example, OBJ for static meshes or a
   supported skeletal format for animation).
5. Convert meshes, normals, UVs, materials and keyframes into a compact
   project-specific runtime format.
6. Validate vertex counts, memory use, winding order and texture dimensions
   on the PS2 target.

## Character parts
- Head and diving mask
- Torso and wetsuit
- Left/right arms
- Left/right legs
- Fins

## Animation clips to author
- Idle / neutral float
- Swim
- Dive
- Rise
- Turn
- Explore

No original-game model, texture, rig or animation is included in this scaffold.
