#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

#define SCREEN_SCE  15   // Pin del chip select (CS)
#define SCREEN_RST  2   // Pin de reset
#define SCREEN_DC   4   // Pin de comando/datos
#define SCREEN_SDIN 23  // Pin de entrada de datos (MOSI)
#define SCREEN_SCLK 18  // Pin de reloj (SCLK)

#define POT_PIN     34  // Pin analógico para el potenciómetro
#define SIGNAL_PIN  36  // Pin analógico para la señal

#define BUTTON_PIN 32  // Pin donde está conectado el botón

// Número de puntos en la pantalla
#define NUM_POINTS  84

// Factor de escala para ajustar la amplitud de la señal
#define SCALE_FACTOR 2

Adafruit_PCD8544 display = Adafruit_PCD8544(SCREEN_SCLK, SCREEN_SDIN, SCREEN_DC, SCREEN_SCE, SCREEN_RST);

int signalValues[NUM_POINTS]; // Almacena los valores de la señal

// Definiciones de señales
#define SINE_WAVE 1
#define SQUARE_WAVE 2

int signalType = SINE_WAVE; // Tipo de señal inicial: senoidal

int generateSignal() {
  static float angle = 0;

  switch (signalType) {
    case SINE_WAVE:
      // Generar señal senoidal
      return (int)(2047 * sin(angle)); // Escalar la señal a 0-4095
      break;
    case SQUARE_WAVE:
      // Generar señal cuadrada
      return angle < PI ? 4095 : 0; // Alternar entre 0 y 4095
      break;
    default:
      return 0;
      break;
  }
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  SerialBT.begin("ESP32_BT");

  display.begin();
  display.setContrast(60);


  // Inicializa el array de valores de la señal
  for (int i = 0; i < NUM_POINTS; i++) {
    signalValues[i] = 0;
  }
}

void loop() {
  int valorPotenciometro = analogRead(34); // Cambiado a pin 34
  SerialBT.println(valorPotenciometro);
  delay(1000);
  
  int buttonState = digitalRead(BUTTON_PIN);

   if (buttonState == LOW) {
    delay(50); // Retardo para evitar rebotes del botón
    if (digitalRead(BUTTON_PIN) == LOW) { // Verificar que el botón sigue presionado después del retardo
      // Cambiar el tipo de señal
      if (signalType == SINE_WAVE) {
        signalType = SQUARE_WAVE; // Cambiar a señal cuadrada
      } else {
        signalType = SINE_WAVE; // Cambiar a señal senoidal
      }
      // Esperar hasta que se libere el botón
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
  }

  // Lee el valor del potenciómetro para ajustar la escala vertical
  int scaleValue = analogRead(POT_PIN);
  float scaleFactor = map(scaleValue, 0, 4095, 1, 10) / 10.0;

  // Lee el valor de la señal analógica
  int signalValue = analogRead(SIGNAL_PIN);
  
  // Agrega el valor al array de la señal
  for (int i = 0; i < NUM_POINTS - 1; i++) {
    signalValues[i] = signalValues[i + 1];
  }
  signalValues[NUM_POINTS - 1] = signalValue;

  // Limpia la pantalla
  display.clearDisplay();

  // Dibujar ejes X e Y
  display.drawLine(0, display.height() / 2, display.width(), display.height() / 2, BLACK); // Eje X
  display.drawLine(display.width() / 2, 0, display.width() / 2, display.height(), BLACK); // Eje Y

  // Dibujar valores de escala en el eje X
  for (int i = 0; i < display.width(); i += 10) {
    // Establece el tamaño del texto para los valores de escala
    
    display.setCursor(i, display.height() / 2 + 5);
    display.print(i);
  }

  // Dibujar valores de escala en el eje Y
  for (int i = 0; i < display.height(); i += 10) {
    // Establece el tamaño del texto para los valores de escala
     
    display.setCursor(display.width() / 2 + 5, i);
    display.print(i);
  }

  // Dibuja la señal en la pantalla
  for (int i = 0; i < NUM_POINTS - 1; i++) {
    int y1 = map(signalValues[i] * scaleFactor, 0, 4095, display.height() / 2, 0);
    int y2 = map(signalValues[i + 1] * scaleFactor, 0, 4095, display.height() / 2, 0);
    display.drawLine(i, y1, i + 1, y2, BLACK);
  }

  // Muestra la pantalla
  display.display();

  delay(50);  // Ajusta el retardo según sea necesario
}