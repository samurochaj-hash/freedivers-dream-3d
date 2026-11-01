# Orçamento inicial de texturas

Use texturas pequenas e reutilizáveis para manter o uso de memória previsível.

| Uso | Tamanho inicial sugerido | Formato candidato |
|---|---:|---|
| Rocha e areia | 128×128 | INDEX8 ou RGBA16 |
| Coral | 128×128 | INDEX8 ou RGBA16 |
| Traje do mergulhador | 256×256 | RGBA16 |
| Peixes pequenos | 64×64 | INDEX8 com paleta |
| Ícones de interface | 64×64 | INDEX8 ou RGBA16 |
| Bolhas | 32×32 | RGBA16 com transparência |

Estas são metas de produção, não assets incluídos. Testar qualidade, paletas e custo
de transferência no hardware/emulador antes de fixar os formatos.
