#include <Wire.h> 
#include <LiquidCrystal_I2C.h> // used for lcd I2C interface

void clr(int line, int col);
void prints(float distance);

LiquidCrystal_I2C lcd(0x27,16,4);  // set the LCD address to 0x27 for a 16 chars and 4 line display
// defines pins numbers
const int trigPin = 9;
const int echoPin = 10;
const int relayPin = 11;
const int overridePin = 12;
// defines variables
long duration;
float distance;
void setup()
{
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(relayPin, OUTPUT); // sets the relay trigger pin as output
  pinMode(overridePin, INPUT); // sets the manual override pin as input
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  Serial.begin(9600);
  digitalWrite(relayPin, LOW); // turns actuator off
}

void loop()
{
    // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.017;
  if (distance > 20.0)
  {
    digitalWrite(relayPin, HIGH); // turns actuator on.
    prints(distance);
    clr(3, 0);
    if (digitalRead(12) || relayPin)
    {
      lcd.print("ON");
    }    
  }
  else
  {
    digitalWrite(relayPin, LOW);
    prints(distance);
    clr(3, 0);
    if (digitalRead(12))  // reads the state of the manual override pin
    {
      lcd.print("ON");
    }
    else
    {
      lcd.print("OFF");
    }
  }
  
  delay(1000);
}

/**
 * prints - prints recursive strings on the LCD display
 */
void prints(float distance)
{
  lcd.setCursor(0, 0); // Sets the location at which subsequent text written to the LCD will be displayed
  lcd.print("DISTANCE: "); // Prints string "Distance" on the LCD
  clr(1, 4);
  lcd.print(distance); // Prints the distance value from the sensor
  lcd.print(" cm");
  lcd.setCursor(0, 2);
  lcd.print("PUMP STATE: ");
      // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
}

/*
* clr - clear the specified @line of the LCD display and sets the cursor at the start of the line
* @line: the line of the LCD display to be cleared.
* @col: where to set the cursor within the specified @line.
*/
void clr(int line, int col)
{               
        lcd.setCursor(0, line);
        for(int n = 0; n < 16; n++) // 16 indicates symbols in line. For 2x20 LCD write - 20
        {
                lcd.print(" ");
        }
        lcd.setCursor(col, line);
}
