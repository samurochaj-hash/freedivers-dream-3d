# Etapa 22 — Sistema de iluminação 3D

## Incluído no protótipo
- Cor e intensidade de iluminação ambiente submarina.
- Ajuste progressivo de névoa/cor conforme a profundidade.
- Lanterna direcional com alcance, cone e atenuação por distância.
- Avaliação simplificada da contribuição de luz em um ponto e normal.

## Integração prevista
1. Chamar `Lighting_Init()` na inicialização.
2. Atualizar a profundidade com `Lighting_SetDepth()`.
3. Atualizar posição e direção da lanterna com `Lighting_SetFlashlight()`.
4. Avaliar a iluminação de superfícies com `Lighting_Evaluate()`.
5. Converter os resultados para cores/atributos aceitos pelo renderizador GSKit.

## Limitações
Este módulo calcula valores de iluminação, mas ainda não envia comandos ao Graphics
Synthesizer. Não implementa sombras reais, mapas de sombra, materiais completos ou
iluminação por pixel. Em hardware PS2, a integração final deverá priorizar iluminação
por vértice, cores pré-calculadas e poucos efeitos simultâneos.
