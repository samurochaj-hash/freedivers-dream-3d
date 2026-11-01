# Etapa 18 — Sistema de exploração e objetivos

## Incluído no protótipo
- Até 12 objetivos por fase.
- Até 32 descobertas registradas.
- Até 24 itens coletáveis registrados.
- Objetivos de descobrir, coletar, alcançar ponto e concluir fase.
- Cálculo de porcentagem de objetivos concluídos.
- Verificação de conclusão da missão.

## Integração prevista
1. Chamar `Exploration_Init()` ao iniciar a fase.
2. Cadastrar objetivos com `Exploration_AddObjective()`.
3. Ao visitar um ponto, chamar `Exploration_RegisterDiscovery()` ou
   `Exploration_ReachPoint()`.
4. Ao pegar um item, chamar `Exploration_CollectItem()`.
5. Ao concluir os requisitos da fase, chamar `Exploration_SetPhaseComplete()`.
6. Consultar `Exploration_IsMissionComplete()` e
   `Exploration_GetCompletionPercent()` para atualizar a interface.

## Limitações
O módulo registra progresso em memória; ainda não salva em Memory Card.
Não há interface visual, mapa de objetivos ou persistência entre sessões.
A lógica precisa ser integrada ao fluxo real das fases e testada no PS2SDK/PCSX2.
