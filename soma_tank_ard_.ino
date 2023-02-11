#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>

LiquidCrystal_I2C lcd(0x27,16,4);  // set the LCD address to 0x27 for a 16 chars and 4 line display
WiFiClient espClient;
PubSubClient client(espClient);

// defines pins numbers
#define trigPin 18 // ultrasonic trigger pin
#define echoPin 19 // ultrasonic echo pin
#define relayPin 5 //relay trigger pin
#define overridePin 17 // manual override pin

// defines variables
long duration;
float distance;
long lastMsg = 0;
long prevTime = 0;
byte pumpStatus = LOW;

const char* ssid = "kinko";
const char* password = "the quadzilla";
const char* mqtt_server = "91.121.93.94";

void setup()
{
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(relayPin, OUTPUT); // sets the relay trigger pin as output
  pinMode(overridePin, INPUT);
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  Serial.begin(115200);
  digitalWrite(relayPin, LOW); // turns actuator off
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect(); // to make sure the connection was successful.
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
  while (!client.connected())
  {
    client.connect("espClient");
  }

  client.subscribe("esp32/output");
  client.loop();
   long now = millis();
   if (now - lastMsg >= 1000) {
        lastMsg = now;
        // Convert the value to a char array
        char distString[8];
        dtostrf(distance, 1, 2, distString);
        client.publish("esp32/distance", distString);
   }
  if (distance > 20.0 && distance < 400.0)
  {
    //pumpState = HIGH;
    prints(distance);
    clr(3, 0);
    if (digitalRead(17) || relayPin)
    {
       lcd.print("ON");        
    }
   }

    else
    {
      //pumpState = LOW;
      prints(distance);
      clr(3, 0);
      if (digitalRead(17))
      {
        lcd.print("ON");
      }
      else
      {
        lcd.print("OFF");
      }
     }
    client.setCallback(callback);
    digitalWrite(relayPin, pumpStatus);
    client.loop();
    delay(1500);
}

/**
 * prints - prints recursive strings on the LCD display
 * @distance: the distance to display
 */
void prints(float distance)
{
  lcd.setCursor(0, 0); // Sets the location at which subsequent text written to the LCD will be displayed
  lcd.print("DISTANCE: "); // Prints string "Distance" on the LCD
  clr(1, 4);
  lcd.print(String(distance) + " cm"); // Prints the distance value from the sensor
  //lcd.print(" cm");
  lcd.setCursor(0, 2);
  lcd.print("PUMP STATE: ");
      // Prints the distance on the Serial Monitor
   Serial.println("Distance: " + String(distance) + " cm");   
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

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println("Connecting to " + String(ssid));

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    long now = millis();
      if (now - prevTime > 100) {
        prevTime = now;
        Serial.print(".");
      }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.println("Message arrived on topic: " + String(topic) + ". Message: ");
  String messageDist;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageDist += (char)message[i];
  }
  Serial.println();

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageDist == "on"){
      Serial.println("on");
      pumpStatus = HIGH;
    }
    else if(messageDist == "off"){
      Serial.println("off");
      pumpStatus = LOW;
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("espClient")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.println("failed, rc=" + String(client.state()) + " try again in 5 seconds");
      // Wait 5 seconds before retrying
    }
  }
}
