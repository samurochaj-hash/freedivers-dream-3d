# Freediver's Dream 3D (PS2 homebrew)

Protótipo original de mergulho livre para PlayStation 2. Desça pela corda-guia, toque a placa na profundidade-alvo e volte à superfície antes de apagar. Cada nível sobe o alvo em 5 m (10, 15, 20...).

Sem assets externos: câmera 3D, clipping, névoa, peixes, algas e HUD são todos procedurais (PS2SDK + gsKit).

## Controles

| Botão | Ação |
|---|---|
| Analógico esquerdo / D-pad | Direção (cima = mergulhar, baixo = subir) |
| X | Bater perna (mais velocidade, gasta mais ar) |
| R1 | Compensar os ouvidos |
| START | Iniciar / reiniciar |

Dicas: perto da superfície você flutua para cima; abaixo de ~12 m você afunda sozinho, então deslize em vez de bater perna para economizar ar. A barra EAR enche conforme você desce; compense com R1 antes de chegar ao máximo, senão o ar acaba mais rápido.

## Compilar (ELF + ISO)

Precisa de Docker:

```sh
sh build.sh
```

Gera `FREEDIVE.ELF` e `FreediversDream3D.iso`.

Sem Docker: suba esta pasta para um repositório no GitHub. O workflow em `.github/workflows/build.yml` compila e disponibiliza a ISO em Actions > Artifacts.

Com ps2dev instalado localmente: `make && make iso` (a ISO exige xorriso, genisoimage ou mkisofs).

## Rodar

- **PCSX2:** abra `FreediversDream3D.iso`, ou execute o `FREEDIVE.ELF` direto (Sistema > Executar ELF).
- **PS2 real:** o mais seguro é rodar o `FREEDIVE.ELF` por uLaunchELF ou ps2client. A ISO pode funcionar via OPL, mas não é garantido.

## Estado do projeto

A lógica de jogo e o visual foram validados num simulador no PC (mesmo `main.c`, APIs do PS2 simuladas), com um bot jogando até o nível 5. A compilação com o toolchain real do PS2 e a execução no PCSX2 ainda não foram testadas. Se algo falhar ao compilar (assinaturas do gsKit mudam entre versões), mande a mensagem de erro.
