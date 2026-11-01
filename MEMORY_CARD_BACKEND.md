# Backend de Memory Card — integração planejada

O módulo `save_system.c` já prepara os dados, calcula checksum e valida o conteúdo
antes de aceitar um carregamento. Ele ainda não acessa diretamente o Memory Card.

## Próxima integração PS2SDK
- Implementar um backend separado usando as funções de I/O de Memory Card da versão
  instalada do PS2SDK.
- Verificar presença do cartão, porta e slot.
- Verificar espaço livre e estado do dispositivo.
- Criar/abrir o arquivo de save e tratar erros de leitura/escrita.
- Fechar o arquivo e confirmar o resultado da operação.
- Não interromper o jogo se o cartão estiver ausente.

## Segurança dos dados
- Manter versão e assinatura do formato.
- Validar tamanho e checksum antes de usar dados carregados.
- Preservar um arquivo anterior válido até confirmar a gravação nova, quando o backend
  permitir estratégia de arquivo temporário/backup.
- Planejar migração quando o formato de save mudar.
