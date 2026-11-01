# Etapa 19 — Sistema de salvamento

## Incluído no protótipo
- Estrutura versionada para fases, descobertas, colecionáveis e tempo de jogo.
- Assinatura de arquivo e checksum CRC-32.
- Funções para marcar progresso.
- Exportação e importação binária com validação.
- Rejeição de assinatura, versão, tamanho ou checksum inválidos.

## Integração prevista
1. Criar `FDSaveData` e chamar `FDSave_Init()`.
2. Atualizar os campos por meio das funções `FDSave_Mark...()`.
3. Reservar um buffer com `sizeof(FDSaveData)`.
4. Exportar com `FDSave_Export()` e enviar os bytes ao backend de Memory Card.
5. Ler os bytes do cartão e validar com `FDSave_Import()`.

## Limitações
Este módulo não implementa chamadas reais ao Memory Card, seleção de slot,
formatação, ícones do save, telas de confirmação ou testes em console.
O formato binário atual é um ponto de partida e pode precisar de ajustes de alinhamento
e compatibilidade conforme compilador/toolchain PS2SDK.
