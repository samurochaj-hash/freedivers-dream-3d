# Fluxo de objetos até o renderizador

1. Carregar ou construir uma malha `RenderMesh`.
2. Criar um `RenderObject` e atribuir malha e material.
3. Atualizar posição, rotação, escala e visibilidade.
4. Chamar `RenderQueue_Add()` para incluir o objeto do frame.
5. Iniciar o frame com as matrizes view/projection.
6. Submeter a fila usando um callback de backend GSKit.
7. Encerrar o frame e sincronizar a apresentação no backend.

A fila não copia os dados da malha: os ponteiros precisam permanecer válidos durante
a submissão do frame.
