#define FLEX1 39
#define FLEX2 36



float impresion1 = 0;
float impresion2 = 0;
float tension = 0;
float flex = 0;
float angulo = 0;

float V_min = 0.5;  // Sustituir con el voltaje medido a 0 grados
float V_max = 1.25;  // Sustituir con el voltaje medido a 180 grados
float V_90 = 0.85;


void setup() {
  pinMode(FLEX1, INPUT);
  pinMode(FLEX2, INPUT);

  Serial.begin(9600);
}

void loop() {

  flex = analogRead(FLEX2);
  tension = obtenerTension(flex);
  obtenerAngulo(tension);

  //delay(500);

  //  Serial.print("Flex 50k: ");
  // Serial.println(impresion1);

  /*
    impresion2 = analogRead(FLEX2);
    Serial.print("Flex 30k: ");
    Serial.println(impresion2);
  */
}


float obtenerTension(float flex) {
  float tension = 0;
  tension = (flex / 4096) * 3.3;
  //Serial.print("tensión: ");
  //Serial.println(tension);
  return tension;
}

float obtenerAngulo(float tension) {

  //angulo = ((tension - 0.997) / (1.87 - 0.997)) * 180;
  //((tension * (-90)) + 180);
  if (tension >= V_min && tension <= V_max) {
    // Cálculo del ángulo usando interpolación lineal
    angulo = (tension - V_min) / (V_max - V_min) * 180;
  } else {
    angulo = 0; // Valor fuera de rango o error
  }

  Serial.print("Angulo: ");
  Serial.println(angulo);

  return angulo;
}