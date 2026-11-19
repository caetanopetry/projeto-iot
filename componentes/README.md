# Lista de componentes

- [`lista_componentes_prototipo_real.csv`](lista_componentes_prototipo_real.csv): inventário a utilizar como referência para a **montagem física**, conforme documentação revisada. Confirme quantidades e modelos na bancada.
- [`bom_tinkercad_original.csv`](bom_tinkercad_original.csv): exportação **original** do Tinkercad, arquivada para rastreabilidade. Não representa a lista definitiva do protótipo físico.

## Ressalvas importantes

**Sensor:** o Tinkercad exportou “Sensor de umidade do solo”. Na montagem real utiliza-se **sensor resistivo de nível de água, com três terminais S, + e -**, fotografado em `../imagens/sensor-nivel-agua-real.png`.

**Teclado:** o Tinkercad contém um **4×4**, mas o projeto físico utiliza **4×3**, conectado conforme o `.ino`.

**LCD:** o item exportado pelo Tinkercad indica **0x20**, enquanto o código físico usa **0x27**. Confirme o endereço efetivo no módulo montado.

**Interruptor DIP:** está presente no CSV exportado, mas não é referenciado pelo código fornecido. A existência ou necessidade no protótipo físico deve ser conferida antes de incluí-lo na lista real.
