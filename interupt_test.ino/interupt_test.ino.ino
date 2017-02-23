void setup() {
  Serial.begin(9600);
  while(!Serial);

  Serial.println("hello");

  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);


  attachInterrupt(A3, onA3Change, CHANGE);
  attachInterrupt(A4, onA4Change, CHANGE);

}

void loop() {
}


void onA3Change() {
  Serial.println("A3 change");
}

void onA4Change() {
  Serial.println("A4 change");
}


