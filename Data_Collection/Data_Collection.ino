//Libraries
#include <DHT.h>;

//DHT sensor constants
#define DHTPIN 2 
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE); //Initialize DHT sensor

//Soil Moisture Sensor Variables
int soilPin = A0; //Pin for reading data
int soilPower = 7;  //Digital pin 7 to toggle on and off

//Soil Moisture function
double readSoil();

void setup() {

  //Begin bluetooth and DHT sensor
  Serial.begin(9600);
  dht.begin();

  //Set soilPower as output and start as off
  pinMode(soilPower, OUTPUT);
  digitalWrite(soilPower, LOW);

}

void loop() {

  float humidity=0;
  float temp=0;
  double moisture = 0;
  int counter;

  //Read dht data
  humidity = dht.readHumidity();
  temp = dht.readTemperature();
  moisture = readSoil();

  //Print temp and humidity values to modity
  Serial.print(humidity);
  Serial.print(",");
  Serial.print(temp);
  Serial.print(",");
  Serial.print(moisture);
  Serial.println(";");

  for(counter = 0; counter < 2; counter++) //Wait 10 minutes
    delay(30*1000);  //Wait half second;

}

  double readSoil(){
  double val=0;
  
  digitalWrite(soilPower, HIGH);  //Turn on sensor
  delay(10);
  val = analogRead(soilPin);  //Read sensor data
  digitalWrite(soilPower, LOW);   //Turn off sensor

  //After experimenting, I see that the highest value when in a cup of water (100%) is 894
  //Therefore the moisture percentage will be the analog reading from the pin/894*100

  val = val*100/894;

  return val;
}

