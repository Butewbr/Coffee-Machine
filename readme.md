# ☕ Cafeteira Inteligente
Autor: Bernardo Pandolfi Costa (19207646)

Disciplina: Microprocessadores e Microcontroladores

## 💡 Legendas das Cores dos LEDs
### Entradas
* LED-AQUA: configurações do café
* LED-RED: possíveis problemas críticos
* LED-PURPLE: sensores que requerem espera
* LED-AMARELO: botões de início/cancelamento

### Saídas
* LED-PINK: Motores/válvulas
* LED-GREEN: Indicador de café pronto

## 🔍 Elementos do Sistema
### Entradas:
* Switch que configura **quente/gelado**
* Switch que configura se terá **leite** ou não
* Sensor de **peso de café**
* Sensor de **volume de água**
* Sensor de **volume de leite**
* Sensor de **Temperatura**

### Saídas:
* Resistência para esquentar a máquina
* Relé para bobina do **café** (água + café)
* Relé para bobina do **leite** X
* Luz indicadora do café pronto

### Watch Dog Time:
* Vai verificar se a cafeteira demora muito pra esquentar
* Se alguma coisa der errado no timer nos despejamentos

### Timer:
* O timer vai controlar o volume de café e de leite que serão despejados, contando em segundos.

### Display de LCD:
* O display de LCD vai ser usado para exibir informações sobre as configurações selecionadas pelo usuário, como tipo de café: gelado ou quente, com leite ou sem, ou alertas de falta de ingredientes.

### Interrupção:
* A interrupção será usada caso o usuário se arrependa de uma de suas escolhas e decida cancelar o processo do café.