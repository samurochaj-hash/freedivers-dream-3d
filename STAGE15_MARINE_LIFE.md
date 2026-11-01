# Etapa 15 — Animais marinhos e vida submarina

## Entregue nesta etapa
- Estrutura de dados para até 24 entidades marinhas simultâneas.
- Tipos: peixe, tartaruga, golfinho e tubarão.
- Movimento básico em 2D no plano horizontal, com estabilização vertical.
- Reação simples de afastamento quando o mergulhador se aproxima.
- Documento de pipeline para criação e integração de modelos e animações.

## Integração
O jogo deve chamar `MarineLife_Init()` ao iniciar a fase, adicionar animais com
`MarineLife_Add()` e atualizar a simulação com `MarineLife_Update()` a cada frame.
As posições do mergulhador devem ser passadas no mesmo sistema de coordenadas
usado pelo jogo.

## Limitações conhecidas
- A lógica é um protótipo determinístico, não uma IA completa.
- Não há renderização dos animais nem modelos 3D nesta etapa.
- A detecção de proximidade usa distância simplificada.
- É necessário validar desempenho e compatibilidade no toolchain PS2SDK real.
