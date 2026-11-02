# Etapa 30 — Conversor para FDM4 v1

## Uso
Requer Python 3. Execute:

```bash
python tools/fdm4_convert.py examples/sample_character.json output/sample_character.fdm4
```

O conversor recebe um JSON intermediário e grava um arquivo binário FDM4 v1.
O exemplo demonstra uma malha triangular mínima com duas juntas e um clipe.

## Pipeline sugerido
1. Criar rig, malha e animações no software 3D.
2. Exportar os dados necessários para o esquema JSON intermediário.
3. Executar `fdm4_convert.py`.
4. Carregar o `.fdm4` no jogo com `fdm4_load_memory()` da etapa 29.

## Importante
Esta etapa não implementa um plugin direto para Blender, FBX, OBJ ou Maya.
É um conversor JSON -> FDM4 para validar o formato e permitir que um exportador
específico seja desenvolvido depois. O JSON precisa conter matrizes inversas
da bind-pose calculadas pelo software/exportador. O código é uma ferramenta
offline para PC, não roda dentro do PS2.
