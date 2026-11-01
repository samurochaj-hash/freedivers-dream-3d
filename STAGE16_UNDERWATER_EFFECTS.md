# Etapa 16 — Efeitos visuais submarinos

## Incluído
- Sistema reutilizável de partículas com limite de 48 elementos.
- Tipos iniciais: bolhas, sedimentos e respingos.
- Atualização simples de posição, velocidade e tempo de vida.
- Emissão de duas bolhas em um evento de respiração.
- Parâmetros de névoa e intensidade de luz em função da profundidade.

## Integração prevista
1. Chamar `UnderwaterFX_Init()` no início da fase.
2. Chamar `UnderwaterFX_Update()` a cada frame.
3. Chamar `UnderwaterFX_UpdateBreathing()` quando ocorrer um evento de respiração.
4. Consultar `UnderwaterFX_GetDepthSettings()` para obter parâmetros de ambiente.
5. Criar o renderizador GS/gsKit para desenhar as partículas e a névoa.

## Limitações
Esta etapa não implementa shaders, água realista, raios volumétricos ou renderização final.
Os valores são parâmetros iniciais para prototipagem e precisam ser ajustados no jogo.
