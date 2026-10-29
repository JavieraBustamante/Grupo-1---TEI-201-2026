// Pin del sensor de humedad
const int sensorHumedad = 34;

void setup() {

  Serial.begin(115200);

  // Configurar lectura de 12 bits
  analogReadResolution(12);

  Serial.println("CALIBRACION DE HUMEDAD UAI");
  Serial.println("-------------------------");
}

void loop() {

  // 1. Tomar 10 mediciones
  long suma = 0;

  for (int i = 0; i < 10; i++) {

    int valor = analogRead(sensorHumedad);

    suma = suma + valor;

    delay(100);
  }

  // 2. Calcular el promedio
  int lecturaPromedio = suma / 10;

  // 3. Mostrar el resultado
  Serial.print("Lectura del sensor: ");
  Serial.println(lecturaPromedio);

  Serial.println("-------------------------");

  // 4. Esperar antes de repetir
  delay(1000);
}