#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_INA219.h>
#include <SdFat.h>
#include <Servo.h>

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
Adafruit_INA219 ina219;
SdFat sd;
Servo direction_servo;
Servo pitch_servo;

const int SW_pin = 2;
const int X_pin = 0;
const int Y_pin = 1;
const int menu_options = 9;
const int ldr_options = 3;
const int max_servo_speed = 10;

int tl_ldr = A8;
int tr_ldr = A9;
int bl_ldr = A10;
int br_ldr = A11;

int red_pin = 3;
int green_pin = 6;
int blue_pin = 13;

int direction = 90;
int pitch = 90;
int value = 1023;

int red = 0;
int green = 0;
int blue = 0;

long menu = 0;
long ldr = 0;
long servo_speed = 1;
long previous_ms = 0;

float energy = 0;
float servo_voltage = 6;
float battery_voltage = 3.7;

void setup() {
  pinMode(SW_pin, INPUT);
  digitalWrite(SW_pin, HIGH);
  Serial.begin(9600);
  ina219.begin();
  lcd.begin(16, 2);

  direction_servo.attach(4);
  pitch_servo.attach(5);

  pinMode(red_pin, OUTPUT);
  pinMode(green_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);
}

void loop() {
  float current = 0;
  float load_voltage = 0;
  current = ina219.getCurrent_mA();
  value = analogRead(A14);
  load_voltage = value * (5.0 / 1024);

  float power = load_voltage * current;
  energy += (power * (millis() - previous_ms)) / 3600000;
  previous_ms = millis();

  int tl_brightness = analogRead(tl_ldr);
  int tr_brightness = analogRead(tr_ldr);
  int bl_brightness = analogRead(bl_ldr);
  int br_brightness = analogRead(br_ldr);

  if (tl_brightness + bl_brightness > tr_brightness + br_brightness && direction > 0) {
    direction -= servo_speed;
  }
  if (tl_brightness + bl_brightness < tr_brightness + br_brightness && direction < 180) {
    direction += servo_speed;
  }
  if (tl_brightness + tr_brightness > bl_brightness + br_brightness && pitch < 180) {
    pitch -= servo_speed;
  }
  if (tl_brightness + tr_brightness < bl_brightness + br_brightness && pitch > 0) {
    pitch += servo_speed;
  }

  direction_servo.write(direction);
  pitch_servo.write(pitch);

  if (menu == 0) {
    lcd.print("Main Menu");
    lcd.setCursor(0, 1);
    lcd.print("Use Joystick < >");
  } else if (menu == 1) {
    lcd.print("Input Voltage");
    lcd.setCursor(0, 1);
    lcd.print(String(load_voltage) + "V");
    if (load_voltage <= 3) {
      digitalWrite(red_pin, 1);
      digitalWrite(green_pin, 0);
      digitalWrite(blue_pin, 0);
    } else if (load_voltage <= 4) {
      digitalWrite(red_pin, 1);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 0);
    } else if (load_voltage <= 5) {
      digitalWrite(red_pin, 0);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 0);
    } else {
      digitalWrite(red_pin, 0);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 1);
    }
  } else if (menu == 2) {
    lcd.print("Input Current");
    lcd.setCursor(0, 1);
    lcd.print(String(current) + "mA");
    if (current <= 300) {
      digitalWrite(red_pin, 1);
      digitalWrite(green_pin, 0);
      digitalWrite(blue_pin, 0);
    } else if (current <= 750) {
      digitalWrite(red_pin, 1);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 0);
    } else if (current <= 1200) {
      digitalWrite(red_pin, 0);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 0);
    } else {
      digitalWrite(red_pin, 0);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 1);
    }
  } else if (menu == 3) {
    lcd.print("Power Input");
    lcd.setCursor(0, 1);
    lcd.print(String(power) + "mW");
    if (power <= 900) {
      digitalWrite(red_pin, 1);
      digitalWrite(green_pin, 0);
      digitalWrite(blue_pin, 0);
    } else if (power <= 3000) {
      digitalWrite(red_pin, 1);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 0);
    } else if (power <= 6000) {
      digitalWrite(red_pin, 0);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 0);
    } else {
      digitalWrite(red_pin, 0);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 1);
    }
  } else if (menu == 4) {
    lcd.print("Energy Total");
    lcd.setCursor(0, 1);
    lcd.print(String(energy, 3) + "mWh");
    digitalWrite(red_pin, 0);
    digitalWrite(green_pin, 0);
    digitalWrite(blue_pin, 0);
  } else if (menu == 5) {
    lcd.print("Uptime");
    lcd.setCursor(0, 1);
    lcd.print(String(millis() / 1000) + "s");
  } else if (menu == 6) {
    if (analogRead(X_pin) > 750 && ldr > 0) {
      ldr -= 1;
    } else if (analogRead(X_pin) > 750 && ldr == 0) {
      ldr = ldr_options;
    } else if (analogRead(X_pin) < 250 && ldr < ldr_options) {
      ldr += 1;
    } else if (analogRead(X_pin) < 250 && ldr == ldr_options) {
      ldr = 0;
    }
    if (ldr == 0) {
      lcd.print("Top Left LDR");
      lcd.setCursor(0, 1);
      lcd.print(tl_brightness);
    } else if (ldr == 1) {
      lcd.print("Top Right LDR");
      lcd.setCursor(0, 1);
      lcd.print(tr_brightness);
    } else if (ldr == 2) {
      lcd.print("Bottom Left LDR");
      lcd.setCursor(0, 1);
      lcd.print(bl_brightness);
    } else if (ldr == 3) {
      lcd.print("Bottom Right LDR");
      lcd.setCursor(0, 1);
      lcd.print(br_brightness);
    }

    int ldr_total = tl_brightness + tr_brightness + bl_brightness + br_brightness;
    if (ldr_total <= 500) {
      digitalWrite(red_pin, 1);
      digitalWrite(green_pin, 0);
      digitalWrite(blue_pin, 0);
    } else if (ldr_total <= 1000) {
      digitalWrite(red_pin, 1);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 0);
    } else if (ldr_total <= 2000) {
      digitalWrite(red_pin, 0);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 0);
    } else {
      digitalWrite(red_pin, 0);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 1);
    }
  } else if (menu == 7) {
    value = analogRead(A4);
    servo_voltage = value * (5.0 / 1024) * 6 / 5;
    lcd.print("Servo Voltage");
    lcd.setCursor(0, 1);
    lcd.print(String(servo_voltage) + "V");
    if (servo_voltage <= 5) {
      digitalWrite(red_pin, 1);
      digitalWrite(green_pin, 0);
      digitalWrite(blue_pin, 0);
    } else if (servo_voltage <= 5.5) {
      digitalWrite(red_pin, 1);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 0);
    } else if (servo_voltage <= 6) {
      digitalWrite(red_pin, 0);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 0);
    } else {
      digitalWrite(red_pin, 0);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 1);
    }
  } else if (menu == 8) {
    double kelvin = log(10000.0 * ((1024.0 / analogRead(A5) - 1)));
    kelvin = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * kelvin * kelvin)) * kelvin);
    float temperature = ((kelvin - 273.15) * 9.0 ) / 5.0 + 32.0;
    lcd.print("Resistor Temp");
    lcd.setCursor(0, 1);
    lcd.print(String(temperature) + "F");
    if (temperature <= 80) {
      digitalWrite(red_pin, 0);
      digitalWrite(green_pin, 0);
      digitalWrite(blue_pin, 1);
    } else if (temperature <= 95) {
      digitalWrite(red_pin, 0);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 1);
    } else if (temperature <= 110) {
      digitalWrite(red_pin, 0);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 0);
    } else if (temperature <= 125) {
      digitalWrite(red_pin, 1);
      digitalWrite(green_pin, 1);
      digitalWrite(blue_pin, 0);
    } else {
      digitalWrite(red_pin, 1);
      digitalWrite(green_pin, 0);
      digitalWrite(blue_pin, 0);
    }
  } else if (menu == 9) {
    if (analogRead(X_pin) > 750 && servo_speed <= max_servo_speed) {
      servo_speed += 1;
    }
    if (analogRead(X_pin) > 750 && servo_speed > max_servo_speed) {
      servo_speed = 0;
    }
    if (analogRead(X_pin) < 250 && servo_speed >= 0) {
      servo_speed -= 1;
    }
    if (analogRead(X_pin) < 250 && servo_speed < 0) {
      servo_speed = max_servo_speed;
    }
    lcd.print("Servo Speed");
    lcd.setCursor(0, 1);
    lcd.print(String(servo_speed) + " deg/sec");
    digitalWrite(red_pin, 0);
    digitalWrite(green_pin, 0);
    digitalWrite(blue_pin, 0);
  }

  if (analogRead(Y_pin) > 750 && menu < menu_options) {
    menu += 1;
  } else if (analogRead(Y_pin) > 750 && menu == menu_options) {
    menu = 0;
  } else if (analogRead(Y_pin) < 250 && menu > 0) {
    menu -= 1;
  } else if (analogRead(Y_pin) < 250 && menu == 0) {
    menu = menu_options;
  }

  delay(500);
  lcd.clear();
}
