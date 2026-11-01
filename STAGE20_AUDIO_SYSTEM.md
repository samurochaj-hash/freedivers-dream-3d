# Etapa 20 — Sistema de áudio submarino

## Incluído no protótipo
- Até 8 canais lógicos simultâneos.
- Barramentos separados para música, ambiência e efeitos sonoros.
- Controle de volume mestre por barramento e mute.
- Parâmetros de volume e pan por canal.
- Identificadores para bolhas, respiração, animais, cavernas e descobertas.

## Integração prevista
1. Chamar `FDAudio_Init()` durante a inicialização do jogo.
2. Carregar os arquivos de áudio originais/licenciados no backend.
3. Chamar `FDAudio_Play()` para solicitar reprodução.
4. Atualizar volumes com `FDAudio_SetBusVolume()`.
5. Implementar um backend específico para o sistema de áudio do PS2 (SPU2).
6. Encaminhar comandos de parar/reproduzir e liberar recursos corretamente.

## Limitações
Esta etapa não reproduz áudio: os canais são apenas uma camada lógica de controle.
Não há samples, música, efeitos, decodificador ou integração SPU2 implementada.
O backend precisa ser desenvolvido e testado no PS2SDK e no hardware/emulador.
