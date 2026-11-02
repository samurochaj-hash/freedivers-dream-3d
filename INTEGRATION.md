# Integração do FDM4

1. Carregue o arquivo inteiro do dispositivo para RAM usando o sistema de I/O do jogo.
2. Aloque arrays com capacidade para os limites declarados/permitidos.
3. Preencha `FDM4Buffers` e chame `fdm4_load_memory`.
4. Inicialize o player de animação com o esqueleto e clipes importados.
5. A cada quadro: atualize o player, construa a paleta com `skeleton_build_palette`,
   deforme a malha com `skin_deform_vertices` e envie a saída ao renderizador.

O loader fornece os dados de esqueleto e animação, mas o formato ainda exige que
o projeto adapte seus alocadores e conecte o carregamento de arquivos ao CD/DVD.
