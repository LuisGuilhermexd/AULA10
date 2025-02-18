video: https://drive.google.com/file/d/1By4ILCIg6p-L9ZFDBQ4UYCgUtQAETzYu/view?usp=sharing
Instruções:

Os LEDs RGB (vermelho, verde e azul) são controlados por meio de modulação por largura de pulso (PWM), permitindo ajustar o brilho de cada LED individualmente.

O LED vermelho (GPIO 12) e o LED azul (GPIO 13) são controlados pelos valores analógicos do joystick.

O LED verde (GPIO 11) é controlado pelo botão do joystick (GPIO 22), alternando entre ligado e desligado.

O botão A (GPIO 5) desliga todos os LEDs (vermelho, verde e azul) quando pressionado.

O PWM é desativado, e os níveis dos LEDs são definidos como 0.

Um display OLED SSD1306 é utilizado para exibir um quadrado que se move de acordo com a posição do joystick.

A borda do display é atualizada com base no estilo definido pelo botão do joystick.
