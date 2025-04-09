#include <Wire.h> // Necessary for I2C communication
#include <LiquidCrystal.h> // Library for LCD screen

int accel = 0x53; // I2C address for this sensor (from data sheet)
float x, y, z;

bool peak_detected = false;
unsigned long peak_start_time = 0;
unsigned long peak_end_time = 0;
unsigned long peak_length = 0;

int steps = 0;

int threshold = 90000; // threshold above which a signifcant movement has occurred

LiquidCrystal lcd(1, 0, 5, 4, 3, 2); // lcd pins setup

void setup() {
  Serial.begin(9600);
  Wire.begin(); // Initialize serial communications
  Wire.beginTransmission(accel); // Start communicating with the device
  Wire.write(0x2D); // Enable measurement
  Wire.write(8); // Get sample measurement
  Wire.endTransmission();

  lcd.begin(16,2);
  lcd.print("Hey!");
  lcd.setCursor(0,1);
  lcd.print("[Test line 2]");
}

void loop() {
  Wire.beginTransmission(accel);
  Wire.write(0x32); // Prepare to get readings for sensor (address from data sheet)
  Wire.endTransmission(false);
  Wire.requestFrom(accel, 6, true); // Get readings (2 readings per direction x 3 directions = 6 values)
  x = (Wire.read() | Wire.read() << 8); // Parse x values
  y = (Wire.read() | Wire.read() << 8); // Parse y values
  y = (Wire.read() | Wire.read() << 8); // Parse z values
 float totalAccel = sqrt(x*x + y*y + z*z);

 // Serial.print("x = "); // Print values
 // Serial.print(x);
 // Serial.print(", y = ");
 // Serial.print(y);
 // Serial.print(", z = ");
 // Serial.print(z);
 // Serial.print(", sum = ");
  Serial.println(totalAccel);
 
  delay(50); 
  if (totalAccel < threshold) {
    peak_detected = false;
  }

  if (totalAccel > threshold && peak_detected == false) {
    peak_detected = true;
    peak_start_time = millis();
    Serial.println("new HIGH");
  } 
  
  if (totalAccel < threshold && peak_detected == true) {
    peak_detected = false;
    peak_end_time = millis();
    peak_length = peak_end_time - peak_start_time;
    Serial.println("new LOW");
  }

  if (peak_length > 100) {
    steps = steps + 1;
    Serial.println("step counted");
    delay(500);
   // Serial.println(steps);
  }

}