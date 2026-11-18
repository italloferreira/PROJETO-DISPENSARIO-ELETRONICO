# Lista de materiais

## Eletrônica

| Qtd | Componente | Função |
|-----|------------|--------|
| 1 | Arduino Uno | Controlador principal |
| 1 | Leitor biométrico R307S | Autenticação por digital |
| 1 | Teclado matricial 4x4 | Autenticação por senha |
| 1 | Tranca eletrônica 12 V | Trava da porta |
| 1 | Módulo relé | Aciona a tranca (12 V) com sinal do Arduino |
| 1 | Fonte 12 V 3 A | Alimentação do Arduino e da tranca |
| 1 | Buzzer (buzina) | Sinal sonoro (não usado no código atual) |
| 1 | LED verde | Indica acesso liberado |
| 1 | LED vermelho | Indica acesso negado |
| 2 | Resistores de 330 Ω | Limitam a corrente dos LEDs |
| 1 | Protoboard | Ligações |
| - | Jumpers macho/fêmea | Ligações |

## Estrutura

| Item | Especificação |
|------|---------------|
| Material | MDF de 3 mm de espessura, sem acabamento (cru) |
| Chapa | 1 m × 1 m (área de corte de 900 × 900 mm) |
| Quantidade de chapas | 10 (4 do dispensário e 6 das gavetas, 1 por gaveta); o arquivo dos trilhos cabe na sobra de uma chapa de gaveta |
| Corte | A laser, arquivos DXF em [`cad/`](../cad/) |
| Dispensário completo | 75 × 70 × 30 cm (C × A × L) |
| Parte das gavetas | 50 × 70 × 30 cm |
| Parte da eletrônica | 25 × 70 × 30 cm |
| Gavetas | 6, todas com divisória, cada uma com fundo de 285 × 494 mm. `gaveta.dxf` tem 1 gaveta: cortar **6 vezes** |
| Trilhos | 294 × 13 mm, 2 por gaveta (12 no total, `trilhos_e_mais.dxf` cortado 1 vez) |

A lista completa de peças, com medidas e quantidades, está em [`cad/README.md`](../cad/README.md).

## Fixação e ferragens

| Qtd | Item | Uso |
|-----|------|-----|
| 2 | Dobradiça de 38 × 36 mm | Porta |
| - | Parafusos | Fixar as dobradiças e a peça de metal da tranca |
| - | Cola super bonder | União das peças e fixação da tranca |

As gavetas não têm puxador de metal: o rasgo de 22 × 120 mm na frente de cada gaveta funciona como puxador.
