# Pipeline de produção de modelos

1. Modelar mergulhador, fauna e cenários em ferramenta 3D.
2. Reduzir polígonos e separar malhas por material.
3. Criar UVs e texturas dentro do orçamento de memória.
4. Triangular as faces e validar normais.
5. Exportar por um conversor próprio para FDM3 v1.
6. Carregar em buffers de RAM e validar índices.
7. Criar `RenderMesh` e `RenderObject` para o renderizador.

Recomendação: manter modelos simples e reutilizar malhas quando possível. A contagem
de polígonos e o orçamento de VRAM precisam ser medidos no hardware/emulador.
