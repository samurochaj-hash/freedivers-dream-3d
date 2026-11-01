# Animation pipeline

The runtime currently provides animation identifiers and an eight-joint pose
container. It does not yet load or play keyframes.

Next implementation tasks:
- Define a versioned binary clip format.
- Add keyframe sampling and interpolation.
- Add local-to-world joint matrix construction.
- Add skinning or segmented-mesh transforms.
- Connect animation selection to player movement state.
- Profile CPU and memory costs on real PS2 hardware/emulation.

Angles in the current C scaffold are radians.
