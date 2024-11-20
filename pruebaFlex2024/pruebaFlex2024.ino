#define FLEX1 36
#define FLEX2 39

float tension = 0;
float angulo = 0;

// Voltage values corresponding to the angles
float V_min = 0.2;  // Voltage at 180 degrees
float V_max = 2.4;  // Voltage at 0 degrees

void setup() {
  pinMode(FLEX1, INPUT);
  pinMode(FLEX2, INPUT);

  Serial.begin(9600);
}

void loop() {
  float flex = analogRead(FLEX1);
  tension = obtenerTension(flex);
  angulo = obtenerAngulo(tension);
  delay(500);  // Delay for readability
}

float obtenerTension(float flex) {
  // Convert the analog reading (0-4095) to a voltage (0-3.3V)
  float tension = (flex * 3.3) / 4095;
  //Serial.print("Tension: ");
  //Serial.println(tension);
  return tension;
}

float obtenerAngulo(float tension) {
  float angulo;

  // Ensure tension is within the expected range for reliable interpolation
  if (tension >= V_min && tension <= V_max) {
    // Calculate the angle using linear interpolation
    angulo = (V_max - tension) / (V_max - V_min) * 180;
  } else {
    angulo = 0;  // Out of range or error
  }

  Serial.print("Angulo: ");
  Serial.println(angulo);

  return angulo;
}
