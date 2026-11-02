# Etapa 27 — Sistema de Skinning 3D

Implementa uma base de Linear Blend Skinning na CPU:
- até quatro influências de juntas por vértice;
- normalização dos pesos positivos;
- deformação de posições e normais;
- paleta de matrizes 4x4 fornecida pelo sistema de esqueleto;
- buffers fornecidos pelo chamador, sem alocação dinâmica.

## Integração
1. O sistema de animação da etapa 26 atualiza as juntas.
2. Um futuro compositor calcula as matrizes globais e a paleta
   (matriz global da junta × inversa da bind-pose).
3. O importador fornece índices e pesos por vértice.
4. `skin_deform_vertices()` produz o buffer deformado.
5. O renderizador usa esse buffer para desenhar a malha.

## Limitações
A etapa não calcula hierarquia global nem inversas da bind-pose, não cria pesos,
não atualiza o formato FDM3, não lê arquivos Blender/Maya e não integra GSKit.
A transformação de normais usa a parte 3x3 da matriz; escala não uniforme exige
matriz normal inversa-transposta. Código base não compilado nem testado no PS2.
