#include <Wire.h> // Necessary for I2C communication
#include <LiquidCrystal.h> // Library for LCD screen

#define BT Serial1  // Pins 0(RX) and 1(TX)

// const SDA = D2; default for Leonardo
// const SCL = D3; default for Leonardo

int accel = 0x53; // I2C address for this sensor (from data sheet)
float x, y, z;

bool move_detected = false;
unsigned long move_start_time = 0;
unsigned long move_end_time = 0;
unsigned long move_length = 0;
unsigned long this_move_time = 0;
unsigned long last_move_time = 0;

int steps = 0;

int threshold = 200; // threshold below which a signifcant movement has occurred

const int rs = 13, en = 12, d4 = 7, d5 = 6, d6 = 5, d7 = 4; // update pins as needed
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); // lcd pins setup

const int buzzer = 8;

int goal = 10; // prevents "goal reached" from triggering immediately

// Helper function to convert HEX character to integer
int hexCharToInt(char ch) {
  if (ch >= '0' && ch <= '9') return ch - '0';
  if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
  if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
  return -1; // invalid HEX character
}

void setup() {
  Serial.begin(115200);    // USB serial monitor
  while (!Serial);         // wait for Serial Monitor
  BT.begin(115200);        // Established baud rate
  Serial.println("Leonardo HC-10 Ready at 115200");

  Wire.begin();
  Wire.beginTransmission(accel);
  Wire.write(0x2D);
  Wire.write(8);
  Wire.endTransmission();

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Waiting BT...");
  pinMode(buzzer, OUTPUT);

  Serial.println("Waiting for step goal via BLE...");

  String hexInput = "";
  char c;

  while (true) {
    if (BT.available()) {
      c = BT.read();

      Serial.print("Received HEX char: 0x");
      if (c < 16) Serial.print("0");
      Serial.println(c, HEX);

      // Only valid HEX chars
      if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
        hexInput += c;
        lcd.setCursor(0, 1);
        lcd.print(hexInput);
      }

      // End input on newline, carriage return, or after 4 hex chars (optional limit)
      if (c == '\n' || c == '\r' || hexInput.length() >= 4) {
        break;
      }
    }
  }

  // Convert HEX input to integer goal
  goal = (int)strtol(hexInput.c_str(), NULL, 16);

  lcd.clear();
  if (goal > 0) {
    lcd.print("Step goal:");
    lcd.setCursor(0, 1);
    lcd.print(goal);
    Serial.print("Goal set to: ");
    Serial.println(goal);
  } else {
    goal = 10;
    lcd.print("Invalid HEX");
    lcd.setCursor(0, 1);
    lcd.print("Using default");
    Serial.println("Invalid HEX input. Goal = 10.");
  }

  delay(2000);
  lcd.clear();
  lcd.print("Begin!");
}

void loop() {
  Wire.beginTransmission(accel);
  Wire.write(0x32); // Prepare to get readings for sensor (address from data sheet)
  Wire.endTransmission(false);
  Wire.requestFrom(accel, 6, true); // Get readings (2 readings per direction x 3 directions = 6 values)
  x = (Wire.read() | Wire.read() << 8); // Parse x values
  y = (Wire.read() | Wire.read() << 8); // Parse y values
  y = (Wire.read() | Wire.read() << 8); // Parse z values
 float totalAccel = sqrt(x*x + y*y + z*z); // calculate magnitude of acceleration vector

 // Serial.print("x = "); // Print values
 // Serial.print(x);
 // Serial.print(", y = ");
 // Serial.print(y);
 // Serial.print(", z = ");
 // Serial.print(z);
 // Serial.print(", sum = ");
  // Serial.println(totalAccel);
 
  delay(50);

  if (totalAccel > threshold && move_detected == true) {
    move_detected = false;
    //move_end_time = millis();
    //move_length = move_end_time - move_start_time;
   // Serial.println("move end"); 

    this_move_time = millis();
    if (this_move_time - last_move_time > 500){ // avoids counting the same step twice
      steps = steps + 1;
     // Serial.print("step count: ");
     // Serial.println(steps);
      lcd.clear();
      lcd.print("Step count: ");
      lcd.print(steps);
      }

    last_move_time = this_move_time; // set current move time to be the time to compare the next one to
  }

  if (totalAccel < threshold && move_detected == false) {
    move_detected = true;
    //move_start_time = millis();
    //Serial.println("move start");
    //move_length = 0;
  }   

  if (steps == goal) {
    tone(buzzer, 440, 250);
    delay(275);
    tone(buzzer, 880, 300);
    delay(400);
    tone(buzzer, 880, 300);
    delay(400);
    tone(buzzer, 880, 300);
    steps = steps + 1; // so tone does not continue playing
  }

  if (steps >= goal) {
    lcd.setCursor(0,1);
    lcd.print("Goal reached!");
  }

  //if (move_length > 100) {
  //  steps = steps + 1;
 //   Serial.println("step counted");
 //   Serial.print("step count: ");
  //  Serial.println(steps);
 // }

}