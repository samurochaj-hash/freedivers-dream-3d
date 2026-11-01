# Dados de animação

Cada AnimClip contém trilhas AnimJointTrack. Cada trilha referencia uma junta
e uma sequência de AnimKeyframe em ordem crescente de time_seconds.
Cada keyframe guarda posição, quaternion e escala locais.

Recomendações: quaternion normalizado, hierarquia sem ciclos, índices de junta
válidos e validação de contagens pelo importador antes de carregar assets.
Os buffers são fornecidos pelo chamador; não há alocação dinâmica.
