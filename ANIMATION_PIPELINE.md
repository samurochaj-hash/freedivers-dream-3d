# Etapa 26 — Sistema de Animação 3D

Inclui estruturas de esqueleto, transformações locais, trilhas por junta,
keyframes temporizados, interpolação de posição/escala e quaternion,
player de clipes, velocidade de reprodução e loop.

Estados planejados: IDLE, SWIM, TURN, ASCEND, DESCEND, FISH_SWIM,
DOLPHIN_SWIM e TURTLE_SWIM. São identificadores; os clipes e keyframes
artísticos ainda precisam ser produzidos.

Integração:
1. Definir o esqueleto em hierarquia.
2. Criar keyframes ordenados por tempo.
3. Montar AnimJointTrack e AnimClip.
4. Inicializar AnimPlayer com buffer de pose de pelo menos joint_count.
5. Chamar anim_player_update a cada quadro.
6. Enviar local_pose à futura composição de matrizes e ao skinning.

Limitações: o formato FDM3 da etapa 25 não possui pesos por vértice. Este
módulo não deforma a malha e ainda não inclui blend entre clipes, IK,
retargeting, conversor de Blender/Maya, assets artísticos ou integração GSKit.
Código scaffold portátil em C; não compilado nem testado no PS2 nesta etapa.
