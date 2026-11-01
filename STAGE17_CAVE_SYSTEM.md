# Etapa 17 — Sistema de cavernas submarinas

## Incluído no protótipo
- Estruturas para até 8 salas, 32 paredes, 12 portais e 32 objetos.
- Volumes AABB para delimitar salas e colisões provisórias.
- Consulta para saber se um ponto está dentro de uma parede.
- Busca da sala atual e do portal mais próximo do jogador.
- Parâmetros básicos de névoa e iluminação por sala.

## Integração prevista
1. Inicializar `CaveSystem` ao carregar uma fase.
2. Cadastrar as salas, paredes, portais e objetos do mapa.
3. Consultar `CaveSystem_PointBlocked()` durante a movimentação.
4. Usar `CaveSystem_FindRoom()` para aplicar luz e névoa da sala.
5. Usar `CaveSystem_FindPortal()` para identificar passagens entre áreas.

## Limitações
As colisões atuais testam um ponto contra caixas, não o volume completo do personagem.
As paredes são colisores provisórios; ainda não existem malhas 3D, túneis renderizados,
geração de cavernas, navegação avançada ou testes no hardware PS2.
