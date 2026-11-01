# Etapa 25 — Carregamento de modelos 3D

## Incluído no protótipo
- Formato binário próprio `FDM3`, versão 1.
- Leitura de vértices, normais, UVs, cor e índices de triângulos.
- Validação de cabeçalho, versão, limites, tamanho do arquivo e índices.
- Buffers fornecidos pelo chamador: o módulo não faz alocação dinâmica.
- Limites iniciais de 2.048 vértices e 4.096 índices por modelo.

## Integração
1. Ler o arquivo de modelo do dispositivo de armazenamento para RAM.
2. Preparar `RenderVertex[]` e `uint16_t[]` com capacidade suficiente.
3. Chamar `Model_LoadFDM3()`.
4. Construir `RenderMesh` apontando para os arrays preenchidos.
5. Associar a malha a um `RenderObject` e inserir na fila do renderizador.

## Limitações
O carregador não lê OBJ, FBX, glTF ou formatos comerciais diretamente.
É necessário criar um conversor externo para exportar os modelos ao formato FDM3.
Não há animação esquelética, materiais embutidos, compressão, LOD ou renderização.
Os dados são lidos de um buffer já presente na memória; a leitura de CD/DVD ou
Memory Card deve ser implementada em outra camada.
