const int x_jug1 = 34;
const int y_jug1 = 35;
const int ctrl_1 = 32;  // D2
const int ctrl_2 = 33;  // D3
const int ctrl_3 = 25;  // D5
const int ctrl_4 = 26;  // D6

int x_1 = 0;
int y_1 = 0;
int x_2 = 0;
int y_2 = 0;
int pos_x_1 = 0;
int pos_y_1 = 0;
uint8_t jugador1 = 0;
uint8_t jugador2 = 0;
 
void setup() {
  pinMode(x_jug1, INPUT);
  pinMode(y_jug1, INPUT);
  pinMode(ctrl_1, INPUT);
  pinMode(ctrl_2, INPUT);
  pinMode(ctrl_3, INPUT);
  pinMode(ctrl_4, INPUT);
  delay(100);
  Serial2.begin(9600);
  Serial.begin(9600);
  delay(100);
}

void loop() {

  x_1 = analogRead(x_jug1); // Se lee posición x del joystick
  y_1 = analogRead(y_jug1); // Se lee posición x del joystick

  // Definir las 5 posiciones dependiendo del joystick
  if(x_1<1366){
    pos_x_1 = 0;    // derecha
  }
  else if((x_1>1365)&&(x_1<2731)){
    pos_x_1 = 1;    // medio
  }
  else{
    pos_x_1 = 2;    // izquierda
  }

  if(y_1<1366){
    pos_y_1 = 0;    // abajo
  }
  else if((y_1>1365)&&(y_1<2731)){
    pos_y_1 = 1;    // medio
  }
  else{
    pos_y_1 = 2;    // arriba
  }

  // Enviar banderas de control para selección de equipo y cambio de modo 
  if(pos_x_1 == 0){     // derecha
    if(pos_y_1 == 0){   // abajo
      jugador1 = 4;     // derecha y abajo
      Serial.print("D");
    }
    else if(pos_y_1 == 1){  // medio
      jugador1 = 3;         // derecha y medio
      Serial.print("R");
    }
    else if(pos_y_1 == 2){  // arriba
      jugador1 = 1;         // derecha y arriba
      Serial.print("U");
    }
  }
  else if(pos_x_1 == 1){  // medio
    if(pos_y_1 == 0){   // abajo
      jugador1 = 4;     // medio y abajo
      Serial.print("D");
    }
    else if(pos_y_1 == 1){  // medio
      jugador1 = 0;         // medio y medio
      Serial.print("M");
    }
    else if(pos_y_1 == 2){  // arriba
      jugador1 = 1;         // medio y arriba
      Serial.print("U");
    }
  }
  else{  // izquierda
    if(pos_y_1 == 0){   // abajo
      jugador1 = 4;     // izquierda y abajo
      Serial.print("D");
    }
    else if(pos_y_1 == 1){  // medio
      jugador1 = 2;         // izquierda y medio
      Serial.print("L");
    }
    else if(pos_y_1 == 2){  // arriba
      jugador1 = 1;         // izquierda y arriba
      Serial.print("U");
    }
  }

  // Verificar posición enviada por el jugador 2 desde ESP8266
  if((digitalRead(ctrl_1))&&(digitalRead(ctrl_3))){
    jugador2 = 1;
  }
  else if((digitalRead(ctrl_1))&&(!digitalRead(ctrl_3))){
    jugador2 = 2;
  }
  else if((!digitalRead(ctrl_1))&&(digitalRead(ctrl_3))){
    jugador2 = 3;
  }
  else if((digitalRead(ctrl_2))&&(digitalRead(ctrl_4))){
    jugador2 = 4;
  }
  else{
    jugador2 = 0;
  }

  // Si las posiciones son iguales, no fue gol
  if(jugador1 == jugador2){
    Serial2.print("A");
    Serial.print("A");
  }

  // Si las posicones son distintas, fue gol
  else{
    Serial2.print("B");
    Serial.print("B");
  }

  // Envío de bandera de gol o no gol cada 100 mS
  delay(100);
}
