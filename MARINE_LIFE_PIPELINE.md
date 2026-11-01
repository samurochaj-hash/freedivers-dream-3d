# Pipeline de vida marinha

## Tipos previstos
- Peixes tropicais: grupos pequenos, nado em cardume e mudança de direção suave.
- Tartarugas: deslocamento lento e exploração de uma área.
- Golfinhos: nado mais rápido e aproximação curiosa, sem colisão obrigatória.
- Tubarões: patrulha distante; no protótipo, afastam-se quando o mergulhador chega perto.

## Regras para os modelos
1. Modelos low-poly adequados ao orçamento de memória e geometria do PS2.
2. Malha separada em corpo e nadadeiras quando isso facilitar a animação.
3. Texturas pequenas, paletizadas quando apropriado.
4. Origem do modelo posicionada no centro do corpo.
5. Exportar cada animal com escala e orientação documentadas.

## Animações planejadas
- `swim_loop`: ciclo contínuo de nado.
- `turn_left` / `turn_right`: curvas suaves.
- `flee`: aceleração curta para se afastar.
- `hover`: movimento quase parado para tartarugas.

## Estado atual
O código desta etapa implementa apenas entidades abstratas e movimento simplificado em C.
Não inclui malhas 3D, texturas, esqueleto, áudio, cardumes avançados ou IA final.
