# Especificação do formato FDM3 v1

Todos os inteiros e floats são armazenados em little-endian. O arquivo não contém
padding entre campos.

## Cabeçalho — 16 bytes
| Campo | Tipo | Valor/descrição |
|---|---|---|
| magic | u32 | `FDM3` (`0x334D4446`) |
| version | u16 | 1 |
| vertex_count | u16 | quantidade de vértices |
| index_count | u16 | quantidade de índices; múltiplo de 3 |
| reserved | u16 | zero |
| flags | u32 | reservado para uso futuro; v1 ignora |

## Cada vértice — 36 bytes
- position: x, y, z — 3 × f32
- normal: x, y, z — 3 × f32
- uv: u, v — 2 × f32
- color: u32

## Índices
Sequência de `index_count` valores u16. Cada índice deve ser menor que `vertex_count`.
A ordem dos índices define triângulos.

A versão 1 não define tangentes, skinning, animação, materiais nem compressão.
