#define POTE 2
float valorPote = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(POTE, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  valorPote = analogRead(POTE);
  Serial.println(valorPote);
}
