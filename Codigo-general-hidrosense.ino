#include <OneWire.h>
#include <DallasTemperature.h>

// ========================================
// PROYECTO: RIEGO INTELIGENTE UAI
// CODIGO GENERAL CALIBRADO
// ========================================

// 1. PINES DE LOS COMPONENTES

const int sensorHumedad = 34;
const int sensorTemperatura = 4;
const int ledRiego = 2;


// 2. CALIBRACION REAL DE HUMEDAD

const int valorSeco = 3010;
const int valorHumedo = 1745;


// 3. PARAMETROS EXPERIMENTALES DE RIEGO

const int humedadCritica = 50;
const int humedadPreventiva = 70;
const int humedadRecuperada = 85;

const float temperaturaAlta = 25.0;


// 4. TIEMPOS DE DEMOSTRACION

const unsigned long tiempoRiego = 10000;
const unsigned long tiempoReposo = 15000;

const int maxCiclos = 3;


// 5. CONFIGURACION SENSOR TEMPERATURA

OneWire oneWire(sensorTemperatura);
DallasTemperature sensorTemp(&oneWire);


// 6. ESTADOS DEL SISTEMA

enum Estado {
  ESPERANDO,
  REGANDO,
  REPOSANDO,
  BLOQUEADO
};

Estado estado = ESPERANDO;

int ciclosRealizados = 0;
unsigned long inicioEstado = 0;


// ========================================
// CONFIGURACION INICIAL
// ========================================

void setup() {

  Serial.begin(115200);

  analogReadResolution(12);

  pinMode(ledRiego, OUTPUT);
  digitalWrite(ledRiego, LOW);

  sensorTemp.begin();

  Serial.println("SISTEMA RIEGO UAI INICIADO");
  Serial.println("SENSOR DE HUMEDAD CALIBRADO");
}


// ========================================
// FUNCIONAMIENTO PRINCIPAL
// ========================================

void loop() {

  unsigned long ahora = millis();

  // 1. LEER SENSOR DE HUMEDAD
  // Promediar 10 mediciones

  long suma = 0;

  for (int i = 0; i < 10; i++) {
    suma += analogRead(sensorHumedad);
    delay(10);
  }

  int lectura = suma / 10;


  // 2. CONVERTIR A HUMEDAD RELATIVA

  int humedad = map(
    lectura,
    valorSeco,
    valorHumedo,
    0,
    100
  );

  humedad = constrain(humedad, 0, 100);


  // 3. LEER TEMPERATURA

  sensorTemp.requestTemperatures();

  float temperatura =
    sensorTemp.getTempCByIndex(0);


  // 4. VERIFICAR SENSOR DE TEMPERATURA

  if (temperatura == DEVICE_DISCONNECTED_C) {

    estado = BLOQUEADO;

    digitalWrite(ledRiego, LOW);

    Serial.println("ERROR SENSOR TEMPERATURA");
    Serial.println("SISTEMA BLOQUEADO");

    delay(1000);
    return;
  }


  // 5. EVALUAR NECESIDAD DE RIEGO

  bool sueloSeco =
    humedad < humedadCritica;

  bool riesgoPreventivo =
    temperatura >= temperaturaAlta &&
    humedad < humedadPreventiva;

  bool necesitaRiego =
    sueloSeco || riesgoPreventivo;


  // 6. CONTROLAR ESTADOS DEL SISTEMA

  if (humedad >= humedadRecuperada &&
      estado != BLOQUEADO) {

    estado = ESPERANDO;
    ciclosRealizados = 0;

  } else {

    switch (estado) {

      case ESPERANDO:

        if (necesitaRiego) {

          if (ciclosRealizados < maxCiclos) {

            estado = REGANDO;

            ciclosRealizados++;

            inicioEstado = ahora;

          } else {

            estado = BLOQUEADO;
          }
        }
        break;


      case REGANDO:

        if (ahora - inicioEstado >= tiempoRiego) {

          estado = REPOSANDO;

          inicioEstado = ahora;
        }
        break;


      case REPOSANDO:

        if (ahora - inicioEstado >= tiempoReposo) {

          if (ciclosRealizados >= maxCiclos) {

            estado = BLOQUEADO;

          } else if (necesitaRiego) {

            estado = REGANDO;

            ciclosRealizados++;

            inicioEstado = ahora;

          } else {

            estado = ESPERANDO;
          }
        }
        break;


      case BLOQUEADO:

        // No permitir nuevas activaciones
        break;
    }
  }


  // 7. CONTROLAR EL LED

  if (estado == REGANDO) {

    digitalWrite(ledRiego, HIGH);

  } else {

    digitalWrite(ledRiego, LOW);
  }


  // 8. MOSTRAR MEDICIONES

  Serial.println("-------------------------");

  Serial.print("Lectura original: ");
  Serial.println(lectura);

  Serial.print("Humedad calibrada: ");
  Serial.print(humedad);
  Serial.println("%");

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" C");

  Serial.print("Ciclos realizados: ");
  Serial.print(ciclosRealizados);
  Serial.print("/");
  Serial.println(maxCiclos);


  // 9. MOSTRAR ESTADO

  switch (estado) {

    case ESPERANDO:

      Serial.println("SIN SOLICITUD DE RIEGO");
      break;

    case REGANDO:

      Serial.println("RIEGO ACTIVADO");
      Serial.println("LED ENCENDIDO");
      break;

    case REPOSANDO:

      Serial.println("REPOSO - INFILTRACION");
      Serial.println("LED APAGADO");
      break;

    case BLOQUEADO:

      Serial.println("SISTEMA BLOQUEADO");
      Serial.println("REQUIERE REVISION");
      break;
  }

  delay(250);
}