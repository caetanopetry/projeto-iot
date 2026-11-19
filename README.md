# Indicador de Nível de Água com Arduino Uno

![Plataforma](https://img.shields.io/badge/plataforma-Arduino%20Uno-00979D?logo=arduino&logoColor=white)
![Linguagem](https://img.shields.io/badge/linguagem-C%2B%2B%20(Arduino)-blue)
![Status](https://img.shields.io/badge/status-protótipo%20funcional-success)

Protótipo de monitoramento de nível de água para **prevenção de transbordamentos residenciais**. Um sensor de nível envia um sinal analógico ao Arduino, que sinaliza a elevação da água por meio de **LEDs**, **buzzer** e **display LCD**, e permite ajustar o ponto de perigo diretamente pelo **teclado de membrana**, com o valor salvo na memória EEPROM.

![Circuito montado no Tinkercad](imagens/montagem-tinkercad.png)

*Circuito simulado no Tinkercad. No protótipo físico, o sensor de umidade do solo e o teclado 4×4 do simulador são substituídos pelo sensor de nível de água e pelo teclado 4×3 (veja [Simulação e protótipo físico](#simulação-e-protótipo-físico)).*

## Sumário

- [Funcionalidades](#funcionalidades)
- [Componentes](#componentes)
- [Circuito e ligações](#circuito-e-ligações)
- [Funcionamento](#funcionamento)
- [Como executar](#como-executar)
- [Simulação e protótipo físico](#simulação-e-protótipo-físico)
- [Limitações conhecidas](#limitações-conhecidas)
- [Estrutura do repositório](#estrutura-do-repositório)
- [Autores](#autores)

## Funcionalidades

- Leitura contínua do nível de água por sensor analógico (pino `A0`).
- Indicação progressiva por quatro LEDs (25%, 50%, 75% e limite de perigo).
- Alerta sonoro que começa lento e acelera conforme a água sobe.
- Display LCD 16×2 com a leitura bruta, a porcentagem do nível e o estado do som.
- Teclado 4×3 para silenciar o buzzer, reativá-lo e gravar o limite de perigo.
- Limite de perigo mantido na EEPROM, que persiste mesmo com o Arduino desligado.

> Este é um protótipo didático. Os valores dependem da calibração do sensor e **não representam altura em centímetros**.

## Componentes

| Qtd. | Componente | Função |
| :--: | --- | --- |
| 1 | Arduino Uno R3 | Leitura do sensor, processamento e controle |
| 1 | Sensor resistivo de nível de água, 3 pinos (`S`, `+`, `-`) | Gera um sinal analógico conforme a água cobre as trilhas |
| 1 | LCD 16×2 com módulo I2C (endereço `0x27`) | Exibe leitura, porcentagem, estado do som e mensagens |
| 1 | Teclado de membrana matricial 4×3 | Controle do buzzer e gravação do limite |
| 1 | Buzzer | Alerta sonoro, controlado com `tone()` |
| 2 | LED verde | Indicam os níveis de 25% e 50% |
| 1 | LED amarelo | Indica o nível de 75% |
| 1 | LED vermelho | Indica que o limite de perigo foi atingido |
| 4 | Resistor de 220 Ω | Limitam a corrente dos LEDs |
| 1 | Protoboard | Montagem do circuito |
| — | Fios jumper e cabo USB | Conexões e alimentação |

A lista também está disponível em [`componentes/lista_componentes_prototipo_real.csv`](componentes/lista_componentes_prototipo_real.csv).

![Sensor de nível de água utilizado no protótipo](imagens/sensor-nivel-agua-real.png)

*Sensor resistivo de nível de água. Quanto mais água cobre as trilhas, maior o valor lido.*

## Circuito e ligações

| Pino do Arduino | Conexão |
| --- | --- |
| `A0` | Sinal `S` do sensor de nível |
| `5V` | `+` do sensor e `VCC` do LCD |
| `GND` | `-` do sensor, `GND` do LCD, negativo do buzzer e LEDs (via resistor) |
| `A4` (SDA) e `A5` (SCL) | Comunicação I2C com o LCD |
| `D8`, `D7`, `D6`, `D5` | Linhas 1 a 4 do teclado 4×3 |
| `D4`, `D3`, `D2` | Colunas 1 a 3 do teclado 4×3 |
| `D9` | LED verde 1 (com resistor de 220 Ω) |
| `D10` | LED verde 2 (com resistor de 220 Ω) |
| `D11` | LED amarelo (com resistor de 220 Ω) |
| `D12` | LED vermelho (com resistor de 220 Ω) |
| `D13` | Positivo do buzzer |

**Mapa do teclado 4×3:**

| Linha | Teclas | Pino |
| :--: | :--: | :--: |
| 1 | `1` `2` `3` | `D8` |
| 2 | `4` `5` `6` | `D7` |
| 3 | `7` `8` `9` | `D6` |
| 4 | `*` `0` `#` | `D5` |

As colunas 1, 2 e 3 correspondem aos pinos `D4`, `D3` e `D2`. A ordem dos terminais do conector do teclado varia conforme o fabricante, portanto identifique linhas e colunas antes de ligar.

## Funcionamento

### Níveis, LEDs e buzzer

O programa considera `600` como o valor máximo do sensor (lido com ele submerso) e calcula a porcentagem com `map(leitura, 0, 600, 0, 100)`. Os LEDs acendem de forma cumulativa conforme a leitura bruta:

| Leitura | LEDs acesos | Buzzer (se ativo) |
| --- | --- | --- |
| 0 a 149 | Nenhum | Silencioso |
| 150 a 299 | Verde 1 | Silencioso |
| 300 a 449 | Verde 1 e 2 | Silencioso |
| 450 até o limite − 1 | Verde 1 e 2 + amarelo | Bipe lento: 1000 Hz, a cada 500 ms |
| Limite ou mais (padrão 480) | Todos, incluindo o vermelho | Bipes de 2100 Hz que aceleram até a leitura 600 (pausa de 160 ms a 35 ms) |

O limite de perigo começa em **480** (80% de 600). Quando a leitura o atinge, o LCD exibe `PERIGO: 100%`. Esse texto indica que o limite configurado foi alcançado, e não necessariamente que o sensor está totalmente submerso.

### Teclado

| Tecla | Função |
| :--: | --- |
| `1` | Desliga o buzzer |
| `2` | Liga o buzzer |
| `3` | Salva a leitura atual como novo limite de perigo (EEPROM) |
| `#` | Exibe o menu de ajuda no LCD |

As mensagens ficam na tela por 1 segundo. O estado do buzzer não é salvo: ao reiniciar, ele volta ligado.

### Display LCD

- **Linha 1:** leitura bruta do sensor e estado do som (`SOM` ou `MUDO`).
- **Linha 2:** `Nivel: xx%` ou, ao atingir o limite, `PERIGO: 100%`.
- A tela é atualizada a cada 120 ms.

### Limite de perigo na EEPROM

Ao ligar, o Arduino lê o endereço `0` da EEPROM e usa o valor salvo se ele estiver entre 101 e 600. Caso contrário (por exemplo, na primeira execução), mantém o padrão de 480.

## Como executar

**Requisitos:** [Arduino IDE](https://www.arduino.cc/en/software) e as bibliotecas abaixo.

| Biblioteca | Instalação | Uso |
| --- | --- | --- |
| `Wire` | Inclusa no Arduino IDE | Comunicação I2C |
| `EEPROM` | Inclusa no Arduino IDE | Salvar o limite de perigo |
| `LiquidCrystal I2C` | Gerenciador de Bibliotecas | Controle do LCD 16×2 |
| `Keypad` | Gerenciador de Bibliotecas | Leitura do teclado matricial |

**Passos:**

1. Monte o circuito conforme a seção [Circuito e ligações](#circuito-e-ligações).
2. Abra `codigo/Indicador_Nivel_Agua/Indicador_Nivel_Agua.ino` na Arduino IDE.
3. Selecione a placa **Arduino Uno** e a porta serial correta.
4. Compile e envie o código para a placa.

**Calibração:**

1. Com o sensor seco, confirme que os LEDs estão apagados e o buzzer em silêncio.
2. Mergulhe o sensor aos poucos e observe os LEDs acendendo em 150, 300 e 450.
3. Leve a água até o ponto que deve ser considerado perigoso e pressione `3` para gravar o limite.
4. Reinicie o Arduino e confirme que o limite foi mantido.

Se o valor do sensor submerso for diferente de 600 na sua instalação, ajuste a constante `VALOR_MAX_SENSOR` no código.

## Simulação e protótipo físico

**[Abrir o projeto no Tinkercad](https://www.tinkercad.com/things/gcfk48sRPLc-projeto-sa-prevencao-de-alagamentos-residenciaias/editel?returnTo=https%3A%2F%2Fwww.tinkercad.com%2Fdashboard&sharecode=hNlV0GFYj3YyVG4pCysihhe0_Woj0avjWztFcsteBuY)**

A lógica, a pinagem e o comportamento são os mesmos nos dois ambientes. O catálogo do Tinkercad não possui alguns dos modelos usados na maquete, então a simulação usa substitutos:

| Item | Tinkercad | Protótipo físico |
| --- | --- | --- |
| Sensor | Sensor de umidade do solo | Sensor resistivo de nível de água |
| Teclado | 4×4 (teclas `A` a `D` sem uso) | 4×3 |
| LCD I2C | Módulo com endereço `0x20` | Módulo com endereço `0x27` |
| Chave DIP | Presente no esquema, sem função no programa | Não utilizada |

A exportação original de componentes do simulador está em [`simulador/bom_tinkercad_original.csv`](simulador/bom_tinkercad_original.csv) e serve apenas como referência da simulação.

## Limitações conhecidas

- **Mensagens bloqueiam a leitura:** durante os avisos do LCD (1 segundo), o programa usa `delay()` e não lê o sensor nem atualiza LEDs e buzzer.
- **Limite abaixo de 450:** o LED vermelho acende antes de o buzzer começar, pois o som só inicia na leitura 450.
- **Limites extremos:** valores de 100 ou menos não são recuperados após reiniciar (voltam para 480), e um limite de 600 gera divisão por zero no cálculo da pausa do buzzer. Recomenda-se gravar valores entre 101 e 599.
- **Texto `MUDO`:** fica no limite direito da primeira linha do LCD e pode aparecer cortado.
- **Tecla `#`:** o menu aparece ao pressionar a tecla; não há detecção de pressão longa.
- **Medição aproximada:** a leitura varia com a condutividade da água, a salinidade e a oxidação das trilhas do sensor. O protótipo não deve ser o único dispositivo de segurança contra alagamentos reais.

## Estrutura do repositório

```
├── codigo/
│   └── Indicador_Nivel_Agua/
│       └── Indicador_Nivel_Agua.ino    # Firmware do Arduino
├── componentes/
│   └── lista_componentes_prototipo_real.csv
├── docs/
│   └── Documentacao_Indicador_Nivel_Agua.pdf
├── imagens/
│   ├── montagem-tinkercad.png
│   └── sensor-nivel-agua-real.png
├── simulador/
│   └── bom_tinkercad_original.csv
└── README.md
```

A [documentação técnica completa](docs/Documentacao_SA_Indicador_Nivel_Agua.pdf) inclui o esquema de ligações, o funcionamento detalhado, um roteiro de testes e o código-fonte.

## Autores

Projeto desenvolvido para a avaliação prática da unidade curricular **Internet das Coisas** do **SENAI/SC**.

| | |
| --- | --- |
| **Estudantes** | Caetano Gbur Petry e Guilherme Arthur Silveira |
| **Turma** | TDESS V01/2025 |
| **Docente** | Sergio Luiz da Silveira |
