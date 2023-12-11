#include <Servo.h>  //servo library
Servo servo;
const int trig_cap = 7;  // for dustbin cap
const int echo_cap = 6;

const int trig_gar = 9;  // for dustbin full sensor
const int echo_gar = 8;

const int servoPin = 5;
const int buzzer = 12;

//rgb led
const int red = 11;
const int green = 10;
// const int blue = 4;

int alerted = 0;

const int obj_threshold_dist = 20;
const int gar_threshold_dist = 17;

bool cap_opened = 0;


#include <dht.h>  // Include library
#define outPin 4  // Defines pin number to which the sensor is connected

dht DHT;


void close_cap() {
  servo.attach(servoPin);
  for (int i = 180; i >= 86; i--) {
    servo.write(i);
    delay(10);
  }
  servo.detach();
}

String get_stats() {
  int gar_dist = 0;
  for (int i = 0; i < 3; i++) {
    gar_dist += measure(trig_gar, echo_gar) / 3;
    delay(10);
  }

  String isfull = "False";
  if(gar_dist < gar_threshold_dist) isfull = "True";

  int readData = DHT.read11(outPin);

  float t = DHT.temperature;  // Read temperature
  float h = DHT.humidity;     // Read humidity

  String stats = "**BIN STATS:**\nTemperature = " + String(t) + "°C | " + String((t * 9.0) / 5.0 + 32.0) + "°F " + "\nHumidity = " + String(h) + "% \nGarbage full? "+String(isfull);
  // Serial.println("TEST");
  // Serial.println(stats);
  return stats;
}

int measure(int trig, int echo) {
  digitalWrite(trig, LOW);   // Set trig pin to low to ensure a clean pulse
  delayMicroseconds(2);      // Delay for 2 microseconds
  digitalWrite(trig, HIGH);  // Send a 10 microsecond pulse by setting trig pin to high
  delayMicroseconds(10);
  digitalWrite(trig, LOW);  // Set trig pin back to low

  // Measure the pulse width of the echo pin and calculate the distance value
  int pulse_duration = pulseIn(echo, HIGH);  // Read pulse duration
  int distance = pulse_duration / 58.00;     // Calculate the distance
  return distance;
}

void setup() {
  Serial.begin(115200);

  //ultrasensor pins
  pinMode(trig_cap, OUTPUT);
  pinMode(echo_cap, INPUT);
  pinMode(trig_gar, OUTPUT);
  pinMode(echo_gar, INPUT);
  pinMode(buzzer, OUTPUT);

  //rgb
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);

  tone(buzzer, 5000, 200);
  //buzzer pin
  // pinMode(buzzer, OUTPUT);

  //servo motor pins
  servo.attach(servoPin);
  servo.write(86);  //close cap on power on
  delay(100);
  servo.detach();
}


void loop() {

  if (Serial.available() > 0) {
    String incomingString = Serial.readString();

    if (incomingString.indexOf("mcu_send_stats") >= 0) {
      // Serial.println("communicating to esp");
      // Serial.print("\n");
      Serial.println("send_string " + get_stats());
    }
  }

  int obj_dist = 0, gar_dist = 0;

  // findout the obj dist for toggling cap
  for (int i = 0; i < 3; i++) {  //average distance
    obj_dist += measure(trig_cap, echo_cap) / 3;
    delay(10);  //delay between measurements
  }

  for (int i = 0; i < 3; i++) {
    gar_dist += measure(trig_gar, echo_gar) / 3;
    delay(10);
  }

  if (gar_dist < gar_threshold_dist) {
    if (!alerted) {
      Serial.println("send_string Dustbin is full! Consider emptying it to maintain cleanliness and hygiene.");
      alerted = 1;
    }
    digitalWrite(red, 1);
    digitalWrite(green, 0);
  } else {
    alerted = 0;
    digitalWrite(red, 0);
    digitalWrite(green, 1);
  }



  if (!cap_opened and gar_dist > gar_threshold_dist and obj_dist < obj_threshold_dist) {
    tone(buzzer, 1000, 200);  // Send 1KHz sound signal...
    servo.attach(servoPin);
    delay(1);
    servo.write(180);
    cap_opened = 1;
    delay(3000);
    // servo.detach();
  }

  else if (!cap_opened and gar_dist < gar_threshold_dist and obj_dist < obj_threshold_dist) {

    tone(buzzer, 300, 200);  // Send 1KHz sound signal...
    delay(300);
    tone(buzzer, 200, 200);  // Send 1KHz sound signal....
  }

  obj_dist = measure(trig_cap, echo_cap);
  if (cap_opened and obj_dist > obj_threshold_dist) {
    close_cap();
    cap_opened = 0;
    // servo.detach();
    tone(buzzer, 1000, 200);  // Send 1KHz sound signal...
    delay(300);
    tone(buzzer, 1000, 200);  // Send 1KHz sound signal....
  }

  delay(1000);
  // Serial.print("obj_dist");
  // Serial.print(obj_dist);
  // Serial.print("\n");
  // Serial.print("gar_dist");
  // Serial.print(gar_dist);
  // Serial.print("\n");
}