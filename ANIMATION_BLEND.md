# Etapa 29 — Transições entre animações 3D

## O que foi preparado
- Mistura entre duas poses locais com o mesmo número de juntas.
- Interpolação linear de posição e escala.
- Interpolação normalizada de quaternion, escolhendo o caminho angular curto.
- Curva smoothstep para suavizar o início e o fim da transição.
- Duração configurável e consulta do fator de mistura.

## Como integrar
1. Atualize o clipe atual e obtenha sua pose local.
2. Inicie o clipe de destino e obtenha sua pose local.
3. Chame `anim_blend_begin()` com as duas poses e um buffer de saída.
4. A cada quadro, atualize as poses dos clipes e chame `anim_blend_update()`.
5. Use `output_pose` como pose local final para o compositor de esqueleto da etapa 28.
6. Quando a transição terminar, mantenha o clipe de destino como clipe ativo.

## Cuidados
- As duas poses precisam ter a mesma ordem e quantidade de juntas.
- Os buffers são fornecidos pelo chamador e devem permanecer válidos durante a transição.
- O chamador deve atualizar as poses de origem e destino antes da mistura de cada quadro.
- Para trocar de animação durante uma transição, capture a pose resultante atual como
  nova origem, evitando saltos.
- Este módulo mistura poses; não escolhe automaticamente o clipe correto com base
  na velocidade ou direção do personagem.

## Limitações
Ainda não há máquina de estados automática, sincronização de eventos, mistura por
máscara de juntas, blend tree, additive animation, root motion ou integração direta
com o player da etapa 26. Os tipos são independentes e exigem conversão explícita
para `AnimTransform`. Código-base em C, não compilado nem testado no console.
