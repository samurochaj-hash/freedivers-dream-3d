# Etapa 21 — Sistema de câmera 3D

## Incluído no protótipo
- Câmera em terceira pessoa com posição, alvo e vetor vertical.
- Rotação orbital por yaw e pitch.
- Distância ajustável com limites mínimo e máximo.
- Suavização do acompanhamento do jogador.
- Função provisória para detectar câmera dentro de uma caixa de parede.

## Integração prevista
1. Inicializar com `Camera3D_Init()`.
2. Converter os controles analógicos em deltas e chamar `Camera3D_SetOrbit()`.
3. Atualizar com `Camera3D_Update()` a cada frame.
4. Consultar as colisões da fase e chamar `Camera3D_ResolveWall()` ou substituir
   por um raycast robusto.
5. Construir a matriz view usando `position`, `target` e `up` no renderizador 3D.

## Limitações
O módulo calcula posições, mas não constrói a matriz de câmera nem desenha a cena.
A prevenção de atravessamento de paredes é apenas uma correção provisória por AABB;
não é um raycast completo e deve ser integrada ao sistema de cavernas.
