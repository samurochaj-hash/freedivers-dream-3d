# Pipeline de áudio

## Categorias previstas
- Música: trilhas originais de exploração e menus.
- Ambiência: correnteza, água distante e atmosfera de cavernas.
- Efeitos: bolhas, respiração, animais, coleta e descobertas.

## Formatos e integração
Escolher o formato final de acordo com o backend de áudio e as ferramentas
disponíveis na versão do PS2SDK utilizada. Preparar arquivos de áudio originais
ou licenciados, testar duração, taxa de amostragem, tamanho e consumo de memória.

## Regras de mixagem
- Separar música, ambiência e efeitos em barramentos.
- Manter o volume mestre e os volumes individuais ajustáveis.
- Limitar vozes simultâneas e priorizar sons importantes.
- Evitar que efeitos repetitivos fiquem excessivamente altos.
- Fazer fade-in/fade-out no backend final quando suportado.

## Estado atual
O módulo implementa controle lógico de canais, barramentos e volumes.
Não inclui arquivos de áudio, decodificador, reprodução real ou integração SPU2.
