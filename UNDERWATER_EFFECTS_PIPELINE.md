# Pipeline de efeitos visuais submarinos

## Efeitos previstos
- Bolhas: partículas pequenas que sobem e desaparecem.
- Sedimentos: partículas lentas com movimento irregular discreto.
- Raios de luz: geometria translúcida simples, limitada em quantidade.
- Névoa: ajuste de fog e cor conforme a profundidade.
- Superfície: efeito de ondulação e pequenas partículas de entrada na água.

## Recomendações para PS2
1. Preferir partículas em billboard com poucos polígonos.
2. Limitar a quantidade simultânea e reutilizar slots.
3. Evitar excesso de transparências sobrepostas.
4. Usar texturas pequenas e paletizadas quando possível.
5. Desativar efeitos fora da câmera ou muito distantes.
6. Medir o custo no hardware/emulador; os limites deste protótipo são provisórios.

## Estado atual
O módulo implementa apenas dados e atualização simples de partículas, além de parâmetros
numéricos para névoa e iluminação por profundidade. Não desenha partículas, feixes de luz,
água ou transparências. A renderização GS/gsKit precisa ser integrada separadamente.
