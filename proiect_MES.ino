#include <Servo.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#include <NewPing.h>

#define TRIGGER_PIN 9
#define ECHO_PIN 10
#define MAX_DISTANCE 400  // Maximum distance we want to measure (in centimeters).

LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
/*
  se leaga senzorul la pinii 9 pt trig si 10 pentru echo
  se leaga lcd ul: SDA la A4, SDC la A5

*/
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);  // NewPing setup of pins and maximum distance.

Servo myservo;  // create servo object to control a servo
int pos = 0;

enum states { IDLE,
              FIND_DISTANCE,
              GAME_OVER,
              OPEN_BOX,
              TAKE_PRIZE,
              CLOSE_BOX,
              WIN,
              RESTART };
int state = IDLE;
int lives = 3;
int IRvalueD = 0;
long randNumber = random(3, 20);

int MIN_DISTANCE_SENZOR = randNumber - 5;
int MAX_DISTANCE_SENZOR = randNumber + 5;

const int microphone_pin = 2;
const int button_pin = 13;
const int button_reset_pin = 12;
const int ir_pin = 8;
const int servo_pin = 3;
const int led_verde_dist = A2;
const int led_rosu_dist = A3;
const int buzzerPin = A1;

void setup() {
  myservo.write(pos);
  // put your setup code here, to run once:
  lcd.init();       // initialize the lcd
  lcd.backlight();  // Turn on the LCD screen backlight
  Serial.begin(9600);

  pinMode(microphone_pin, INPUT);
  pinMode(button_pin, INPUT_PULLUP);        // pentru buton
  pinMode(button_reset_pin, INPUT_PULLUP);  // pentru buton reset
  pinMode(ir_pin, INPUT);                   // senzor ir

  myservo.attach(servo_pin);

  pinMode(led_verde_dist, OUTPUT);
  pinMode(led_rosu_dist, OUTPUT);

  pinMode(buzzerPin, OUTPUT);

  randomSeed(analogRead(0));  // dai un pin care nu are nimic
}

void loop() {
  // put your main code here, to run repeatedly

  switch (state) {
    case IDLE:
      {
        lcd.setCursor(0, 0);
        lcd.print("Faceti sunet");

        int statusSensor = digitalRead(microphone_pin);

        if (statusSensor == 1) {
          // s-a batut din palme
          lcd.setCursor(0, 0);
          lcd.print("Sunet detectat");
          state = FIND_DISTANCE;
          delay(2000);
          lcd.clear();
        }
      }

      break;
    case FIND_DISTANCE:
      {
        digitalWrite(led_verde_dist, LOW);
        digitalWrite(led_rosu_dist, LOW);

        int distance = sonar.ping_cm();  // Send ping, get distance in cm and print result (0 = outside set distance range)

        lcd.setCursor(0, 0);
        lcd.print("Move to ");
        lcd.print(randNumber);

        lcd.setCursor(0, 1);      // Sets the location at which subsequent text written to the LCD will be displayed
        lcd.print("Distance: ");  // Prints string "Distance" on the LCD
        if (distance < 10)
          lcd.print(" ");
        lcd.print(distance);  // Prints the distance value from the sensor
        lcd.print(" cm");

        // verificam butonul
        int button = digitalRead(button_pin);

        if (button == HIGH)  // s-a apasat
        {
          lives--;
          if (lives < 0) {
            state = GAME_OVER;
            lcd.clear();
            lcd.setCursor(0, 0);
            digitalWrite(led_verde_dist, LOW);
            digitalWrite(led_rosu_dist, HIGH);
            lcd.print("Ai pierdut");
            digitalWrite(buzzerPin, HIGH);
            delay(500);
            digitalWrite(buzzerPin, LOW);
            delay(3000);
            lcd.clear();
            break;
          } else if (chech_distance(distance)) {
            state = OPEN_BOX;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Ai nimerit");
            digitalWrite(led_verde_dist, HIGH);
            digitalWrite(led_rosu_dist, LOW);
            digitalWrite(buzzerPin, HIGH);
            delay(500);
            digitalWrite(buzzerPin, LOW);
            delay(3000);
            lcd.clear();
          } else {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Mai incearca");
            digitalWrite(led_verde_dist, LOW);
            digitalWrite(led_rosu_dist, HIGH);
            digitalWrite(buzzerPin, HIGH);
            delay(500);
            digitalWrite(buzzerPin, LOW);
            delay(1500);
            lcd.clear();
          }
        }
      }

      break;
    case GAME_OVER:
      {

        lcd.setCursor(0, 0);
        lcd.print("GAME OVER");

        digitalWrite(buzzerPin, HIGH);
        delay(500);
        digitalWrite(buzzerPin, LOW);
        delay(500);

        restart_game();
      }
      break;
    case OPEN_BOX:
      {
        digitalWrite(led_verde_dist, LOW);
        digitalWrite(led_rosu_dist, LOW);

        lcd.setCursor(0, 0);
        lcd.print("Open box");

        for (pos = 0; pos <= 80; pos += 1) {  // goes from 0 degrees to 180 degrees
          digitalWrite(buzzerPin, HIGH);

          // in steps of 1 degree
          myservo.write(pos);  // tell servo to go to position in variable 'pos'
          delay(15);           // waits 15 ms for the servo to reach the position
          digitalWrite(buzzerPin, LOW);
        }

        delay(3000);
        lcd.clear();
        state = TAKE_PRIZE;
      }
      break;
    case TAKE_PRIZE:
      {
        lcd.setCursor(0, 0);
        lcd.print("Take prize");
        int IRvalueD = digitalRead(ir_pin);
        if (IRvalueD == 1) {
          delay(3000);
          lcd.clear();
          state = CLOSE_BOX;
        }
      }
      break;
    case CLOSE_BOX:
      {
        lcd.setCursor(0, 0);
        lcd.print("Close box");

        for (pos = 80; pos >= 0; pos -= 1) {  // goes from 180 degrees to 0 degrees
          digitalWrite(buzzerPin, HIGH);
          myservo.write(pos);  // tell servo to go to position in variable 'pos'
          delay(15);
          digitalWrite(buzzerPin, LOW);  // waits 15 ms for the servo to reach the position
        }

        delay(3000);
        lcd.clear();
        state = WIN;
      }
      break;
    case WIN:
      {
        lcd.setCursor(0, 0);
        lcd.print("You won!");
        digitalWrite(buzzerPin, HIGH);
        delay(250);
        digitalWrite(buzzerPin, LOW);
        delay(250);

        restart_game();
      }
      break;
    default:
      // statements
      break;
  }
  delay(1);  // delay in between reads for stability
}

void restart_game() {
  digitalWrite(led_verde_dist, LOW);
  digitalWrite(led_rosu_dist, LOW);

  // verificam butonul
  int button_reset = digitalRead(button_reset_pin);

  if (button_reset == HIGH) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Restarting game");
    lcd.setCursor(0, 1);
    lcd.print("in: ");
    lcd.setCursor(4, 1);
    lcd.print("3");
    lcd.setCursor(4, 1);
    delay(1000);
    lcd.print("2");
    lcd.setCursor(4, 1);
    delay(1000);
    lcd.print("1");
    delay(1000);


    state = IDLE;
    lives = 3;
    IRvalueD = 0;
    randNumber = random(3, 20);
    MIN_DISTANCE_SENZOR = randNumber - 5;
    MAX_DISTANCE_SENZOR = randNumber + 5;

    delay(2000);
    lcd.clear();
  }
}

bool chech_distance(int dist) {
  if (MIN_DISTANCE_SENZOR < dist && dist < MAX_DISTANCE_SENZOR)
    return true;
  return false;
}
