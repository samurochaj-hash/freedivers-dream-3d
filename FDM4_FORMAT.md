# Especificação FDM4 v1

Todos os inteiros e floats são little-endian. Floats são IEEE-754 binary32.
O leitor espera o arquivo sem padding entre campos.

## Cabeçalho (32 bytes)
| Campo | Tipo |
|---|---|
| magic | u32: bytes `FDM4` |
| version | u16: 1 |
| reserved | u16: 0 |
| vertex_count | u32 |
| index_count | u32 |
| joint_count | u16 |
| clip_count | u16 |
| track_count | u16 |
| reserved2 | u16: 0 |
| key_count | u32 |

## Blocos, nesta ordem
1. Vértices: para cada vértice, posição vec3, normal vec3, depois 4 pares
   (joint u16, weight f32).
2. Índices: `index_count` valores u32.
3. Juntas: parent i16, flags u16, transform local (position vec3,
   quaternion xyzw, scale vec3), inverse-bind matrix (16 floats row-major).
4. Trilhas: joint_index u16, key_count u16.
5. Keyframes: agrupados na mesma ordem das trilhas; cada key tem time f32 e
   transform (vec3 + quaternion + vec3).
6. Clipes: nome fixo de 24 bytes, duração f32, joint_count u16,
   track_count u16, first_track u16, looping u8, flags u8 (zero).

## Limites do runtime
- 8.192 vértices; 24.576 índices; 32 juntas; 16 clipes.
- Até 64 keyframes por trilha.
- Juntas em ordem pai-antes-do-filho; raiz usa parent = -1.
- Índices da malha referenciam vértices; índices de junta devem ser válidos.
- Pesos negativos são inválidos; soma positiva é normalizada no carregamento.
