#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <HX711.h>
#include <DHT.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#define CH 2779491
// Wi-Fi credentials
//const char ssid = "";         // Replace with your Wi-Fi name
//const char password = ""; // Replace with your Wi-Fi password
char ssid[] = "Wokwi-GUEST";
// char password[] = "asdasdasd";
unsigned long ch = CH;
// ThingSpeak API Key
const char* apiKey = "5G0NGD1FS8NRSRZ7";    // Replace with your ThingSpeak Write API Key

WiFiClient client;

const int trigPin = 12;
const int echoPin = 14;
const int CDP = 15; //dt
const int CSP = 2; // sck
HX711 scale;

int led = 19;
int irsensor = 18;

// I2C LCD display addresses
const int lcdAddress1 = 0x27; // Replace with the actual I2C address of LCD 1
//const int lcdAddress2 = 0x28; // Replace with the actual I2C address of LCD 2

// I2C LCD displays
LiquidCrystal_I2C lcd1(lcdAddress1, 16, 2);
//LiquidCrystal_I2C lcd2(lcdAddress2, 16, 2);

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
#define DHTPIN 5    
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);
int qt=0;
long duration;
float distanceCm;
float distanceInch;
int chk;
float hum;  
float temp;
int ir;

void setup() {
  // Start serial communication
  Serial.begin(9600);

 // Wi-Fi setup , password
  WiFi.begin(ssid);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");



  scale.begin(CDP, CSP);
  scale.set_scale();

pinMode (led, OUTPUT);
pinMode (irsensor, INPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  dht.begin();

  // Initialize the I2C LCD displays
  lcd1.init();
  // lcd2.init();

  // Backlight the LCD displays
  lcd1.backlight();
  // lcd2.backlight();

  // Clear the LCD displays
  lcd1.clear();
//  lcd2.clear();
  ThingSpeak.begin(client);
}


//Heres loop function
void loop() {
  if (digitalRead(irsensor) == 0)
{
digitalWrite (led, HIGH);
ir=1;
}
else
{
digitalWrite(led, LOW);
ir=0;
}

  float weight = scale.get_units(10) / 420.00;
  Serial.println(weight);
  
  // Display data on LCD 1
  lcd1.clear();
  lcd1.print("Weight: ");
  lcd1.setCursor(0, 1);
  lcd1.print(weight);
  lcd1.print("Kg");


  
  // Delay for a while before taking new readings
  delay(2000);
  lcd1.clear();
  lcd1.setCursor(0,0);
  //Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
  
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);

  // lcd2.clear(); 
  // lcd2.setCursor(0,0);

  if(distanceCm>=29)
    qt=0;
  else if(distanceCm>=24 && distanceCm<=28)
    qt=1;
  else if(distanceCm>=19 && distanceCm<=23)
    qt=2;
  else if(distanceCm>=14 && distanceCm<=18)
    qt=3;
  else if(distanceCm>=8 && distanceCm<=13)
    qt=4;
  else if(distanceCm<7)
    qt=5;

  // lcd2.print("In stock : ");
  // lcd2.print((int)qt);
  // delay(1000);
  // lcd2.clear(); 
  lcd1.print("In stock : ");
  lcd1.print((int)qt);
  delay(1000);
  lcd1.clear(); 



    //Read data and store it to variables hum and temp
    hum = dht.readHumidity();
    temp= dht.readTemperature();
  //  Print temp and humidity values to serial monitor
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %, Temp: ");
    Serial.print(temp);
    Serial.println(" Celsius");
    delay(2000); //Delay 2 sec.

  //  set the cursor on the first row and column 

    lcd1.print("Humidity="); 
    lcd1.print((float)dht.readHumidity());//print the humidity 
    lcd1.print("%"); 
    lcd1.setCursor(0,1);//set the cursor on the second row and first column 
    lcd1.print("Temp="); 
    lcd1.print((float)dht.readTemperature());//print the temperature 
    lcd1.print("C"); 
    delay(1000); 
    lcd1.clear(); 

     // Send data to ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
//    HTTPClient http;
//    String url = server;
//    url += "/update?api_key=" + apiKey;
//    url += "&field1=" + String(weight);            // Weight data
//    url += "&field2=" + String(distanceCm);        // Distance data
//    url += "&field3=" + String(hum);               // Humidity data
//    url += "&field4=" + String(temp);              // Temperature data
//    url += "&field5=" + String(qt);                // Quantity (stock)
//    url += "&field5=" + String(irsensor);


   static unsigned long lastThingSpeakUpdate = 0;
  if (millis() - lastThingSpeakUpdate > 5000) {
    lastThingSpeakUpdate = millis();

    // Get values from readSpeed and getDistance
    ThingSpeak.setField(1, weight);
    ThingSpeak.setField(2, distanceCm);
    ThingSpeak.setField(3, hum);
    ThingSpeak.setField(4, temp);
    ThingSpeak.setField(5, qt);
    if(ir==1){
      ThingSpeak.setField(6,"Motion Detected");
    }
    else{
      ThingSpeak.setField(6,"No one inside");
    }
    

    int responseCode = ThingSpeak.writeFields(ch, apiKey);
    if (responseCode == 200) {
      Serial.println("ThingSpeak Update: Success");
    } else {
      Serial.print("ThingSpeak Update: Failed. Code ");
      Serial.println(responseCode);
    }
  }
    
 
  }

  delay(5000); // ThingSpeak free tier limit (15 seconds minimum)

}
