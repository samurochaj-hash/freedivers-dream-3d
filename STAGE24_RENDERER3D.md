# Etapa 24 — Estrutura inicial de renderização 3D

## Incluído
- Tipos de vértice com posição, normal, UV e cor.
- Descritor de malha com arrays de vértices e índices.
- Objetos com posição, rotação, escala, malha e material.
- Matrizes 4×4: identidade, multiplicação e matriz de mundo.
- Fila limitada de comandos de desenho.
- Callback de submissão para separar a lógica do motor do backend gráfico.

## Integração prevista com PS2
O callback `RendererSubmitFn` será o ponto de ligação com GSKit/Graphics Synthesizer.
O backend deverá configurar textura/material, transformar vértices, enviar primitivas
e controlar buffers e sincronização do GS.

## Limitações
Este módulo ainda não desenha triângulos nem envia comandos ao Graphics Synthesizer.
As matrizes são calculadas na CPU e precisam ser adaptadas/verificadas conforme a
convenção de coordenadas e o backend final. Não há frustum culling, ordenação por
material, clipping, carregador de malhas ou conversor de modelos nesta etapa.
