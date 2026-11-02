# Esquema JSON intermediário

- `vertices`: lista de objetos com `position` [x,y,z], `normal` [x,y,z] e
  `weights`: até quatro pares `[joint_index, weight]`.
- `indices`: lista de índices de vértice, em triângulos (múltiplo de 3).
- `joints`: lista ordenada por hierarquia; cada junta tem `parent` (-1 na raiz),
  `bind_local` (position, rotation quaternion xyzw, scale) e `inverse_bind`
  (16 floats row-major).
- `clips`: lista de clipes com `name` (até 24 bytes UTF-8), `duration`,
  `joint_count`, `looping` e `tracks`.
- Cada track contém `joint` e `keys`; cada key contém `time` e `transform`.

Pesos são normalizados durante a conversão. Os tempos dos keyframes devem ser
crescentes e não exceder a duração do clipe.
