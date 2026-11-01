# Etapa 23 — Sistema de materiais e texturas 3D

## Incluído no protótipo
- Registro de materiais com nome, cor, escala UV e modo de transparência.
- Descritor de textura com dimensões, formato, repetição/limite e filtro.
- Formatos conceituais: RGBA32, RGBA16, índice 8 bits e índice 4 bits.
- Validação básica de descritores e estimativa do tamanho do bitmap.
- Indicação de endereço de textura e paleta para futura integração com GS VRAM.

## Materiais planejados
- Rocha, areia e coral: opacos, com repetição de textura.
- Traje e máscara do mergulhador: opacos, com textura de resolução moderada.
- Peixes e animais: opacos ou recortados, conforme o modelo.
- Bolhas: translúcidas, com uso econômico de camadas.
- Água: efeito simplificado; transparência em tela cheia deve ser evitada ou limitada
  devido ao custo de preenchimento e à memória do PS2.

## Integração necessária
O renderizador deverá converter `TextureDescriptor` para os registradores e estruturas
GSKit apropriados, reservar VRAM, transferir pixels e paletas, configurar filtros e
aplicar os parâmetros de material durante o desenho.

## Limitações
Este módulo não carrega PNG/BMP, não converte imagens, não envia texturas à VRAM e
não renderiza materiais. `Texture_EstimateBytes()` estima apenas os pixels, sem CLUT,
alinhamento, mipmaps ou padding. A validação é preliminar e deverá ser ajustada ao
layout real de VRAM escolhido para o projeto.
