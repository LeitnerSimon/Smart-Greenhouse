#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
#include <Adafruit_BMP280.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "EspMQTTClient.h"

#define LED_PIN_blue 12
#define LED_PIN_green 14     
#define VENT_PIN 27   
#define WATER_PIN 13
#define SERVO_PIN 15
#define SERVO_PIN_vent 18
#define rainpin 32

// DISPLAY
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//Servo Motor
int pos = 0; // variable to store the servo position
bool automatic = false;
bool check1 = false;
bool check2 = false;
bool check3 = false;
bool count_check = false;
bool rain_check = false;
int count = 0;
Servo myservo;

// Display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool vent_count = true;

// some system configuration
EspMQTTClient client(
  "Network", // SSID name
  "12345678", // SSID Password
  "broker.hivemq.com",  // MQTT Broker server ip
  "",   // username: Can be omitted if not needed
  "",   // password: Can be omitted if not needed
  "aau_gh_fd2022",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

const char* ssid = "Network";
const char* password =  "12345678";

// last known timestamp
long last_time = millis();

// time elapsed
float time_elapsed = 1000;

// Methods

int getRainSensor() {
  // Rain Sensor
  int sensorvalue = analogRead(rainpin);
  int sensorvalue2 = map(sensorvalue,4096,0,0,100);
  return sensorvalue2;
} 
int getSoilMoistureSensor() {
  // Soil Moiture Sensor
  int soil_moisturePin = 35;
  int soilMoisturevolts = 0;
  int soilmoisturepercent;

  soilMoisturevolts = analogRead(soil_moisturePin);  //put Sensor insert into soil
  
  soilmoisturepercent = map(soilMoisturevolts, 0, 4096, 0, 100);
    
    return soilmoisturepercent;
      
}
void printAllBMP280_Data() {
  // BMP280 Preasure/Temperature Sensor
  Adafruit_BMP280 bmp; // I2C
  bmp.begin();

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

    Serial.print(F("Temperature = "));
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");

    Serial.print(F("Pressure = "));
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");

    Serial.print(F("Approx altitude = "));
    Serial.print(bmp.readAltitude(1013.25)); /* Adjusted to local forecast! */
    Serial.println(" m");
}
float getBMP280_Temperatur() {
  // BMP280 value Temp
  Adafruit_BMP280 bmp; // I2C
  bmp.begin();

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  return bmp.readTemperature();
}
float getBMP280_Pressure() {
  // BMP280 value Pressure
  Adafruit_BMP280 bmp; // I2C
  bmp.begin();

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

    return (bmp.readPressure()/1000);
}
float getBMP280_ApproxAltidute() {
  // BMP280 value altitude
  Adafruit_BMP280 bmp; // I2C
  bmp.begin();

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

    
   return bmp.readAltitude(1013.25);
    
}
float getSoil_Temperatur() {
  //Soil Temp
  #define SENSOR_PIN  4 // ESP32 pin GIOP21 connected to DS18B20 sensor's DQ pin

  OneWire oneWire(SENSOR_PIN);
  DallasTemperature DS18B20(&oneWire);

  float tempC; // temperature in Celsius

  DS18B20.begin();    // initialize the DS18B20 sensor
  DS18B20.requestTemperatures();       // send the command to get temperatures
  return tempC = DS18B20.getTempCByIndex(0);  // read temperature in °C
}
int getTS2591() {
// TS2591 light Sensor
/* Dynamic Range: 600M:1 */
/* Maximum Lux: 88K */

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

   tsl.begin();
   
   /* Display some basic information on this sensor */
   sensor_t sensor;
   tsl.getSensor(&sensor);
   
   /* Configure the sensor */
   tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  
   tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  
    uint16_t x = tsl.getLuminosity(TSL2591_VISIBLE);
    uint32_t lum = tsl.getFullLuminosity();
    uint16_t ir, full;
    ir = lum >> 16;
    full = lum & 0xFFFF;
    
    return map(tsl.calculateLux(full, ir),0,512,0,100);

}
int getUVM30A() {
  // UV-Sensor
  int UVindex = 0;
  int const UV_SENSOR_PIN = 34;
  
  int sensor_volts = analogRead(UV_SENSOR_PIN);
  float volts = sensor_volts * 5.0 / 1024.0;
  
if (volts < 50)
   {
     UVindex = 0;
   } else if (volts >= 50 && volts < 227)
   {
     UVindex = 1;
   } else if (volts >= 227 && volts < 318)
   {UVindex = 2;} else if (volts >= 318 && volts <408)
   {UVindex = 3;} else if (volts >= 408 && volts < 503)
   {UVindex = 4;} else if (volts >= 503 && volts < 606)
   {UVindex = 5;} else if (volts >= 606 && volts < 696)
   {UVindex = 6;} else if (volts >= 696 && volts < 795)
   {UVindex = 7;} else if (volts >= 795 && volts < 881)
   {UVindex = 8;} else if (volts >= 881 && volts < 976)
   {UVindex = 9;} else if (volts >= 976 && volts < 1079)
   {UVindex = 10;} else if (volts >= 1079)
   {UVindex = 11;}

   return UVindex;

}

void start_vent(boolean on_off){
  if (on_off)
  {
    digitalWrite(VENT_PIN,HIGH);
  } else {
    digitalWrite(VENT_PIN,LOW);
  }
}
void start_water_pump(boolean on_off){
  if (on_off)
  {
    digitalWrite(WATER_PIN,HIGH);
  } else {
    digitalWrite(WATER_PIN,LOW);
  } 
}

// LED
void RGB_LED_red(boolean on) {
  if (on)
  {
    digitalWrite(LED_PIN_green,HIGH);
  } else {
    digitalWrite(LED_PIN_green,LOW);
  }
  
}
void RGB_LED_blue(boolean on) {
  if (on)
  {
    digitalWrite(LED_PIN_blue,HIGH);
  } else {
    digitalWrite(LED_PIN_blue,LOW);
  }
  
}

//DISPLAY
void print_display_data() {

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
  display.print("Rain: ");
  display.println(getRainSensor());
  display.print("Moisture: ");
  display.println(getSoilMoistureSensor());
  display.print("Temperature: ");
  display.println(getBMP280_Temperatur());
  display.print("Pressure: ");
  display.println(getBMP280_Pressure());
  display.print("Soil Temp.: ");
  display.println(getSoil_Temperatur());
  display.print("Light: ");
  display.println(getTS2591());
  display.print("UV-Light: ");
  display.println(getUVM30A());
  display.display(); 
  

}

void end_servo() {

  Servo myservo; 
  
  myservo.attach(SERVO_PIN); // attaches the servo on pin 9 to the servo object

   for(pos = 0; pos <= 70; pos += 1) // goes from 0 degrees to 180 degrees
   { // in steps of 1 degree
     myservo.write(pos); // tell servo to go to position in variable 'pos'
     delay(15); // waits 15ms for the servo to reach the position
   }

}
void start_servo_vent() {

  Servo myservo; 
  
  myservo.attach(SERVO_PIN_vent); // attaches the servo on pin 9 to the servo object

   for(pos = 0; pos <= 100; pos += 1) // goes from 0 degrees to 180 degrees
   { // in steps of 1 degree
     myservo.write(pos); // tell servo to go to position in variable 'pos'
     delay(15); // waits 15ms for the servo to reach the position
   }


}
void start_servo() {
   
  Servo myservo; 
  
  myservo.attach(SERVO_PIN); // attaches the servo on pin 9 to the servo object

   for(pos = 100; pos >= 0; pos -= 1) // goes from 0 degrees to 180 degrees
   { // in steps of 1 degree
     myservo.write(pos); // tell servo to go to position in variable 'pos'
     delay(15); // waits 15ms for the servo to reach the position
   }
}
void end_servo_vent() {
   
  Servo myservo; 
  
  myservo.attach(SERVO_PIN_vent); // attaches the servo on pin 9 to the servo object

   for(pos = 100; pos >= 0; pos -= 1) // goes from 0 degrees to 180 degrees
   { // in steps of 1 degree
     myservo.write(pos); // tell servo to go to position in variable 'pos'
     delay(15); // waits 15ms for the servo to reach the position
   }
}

void start_ventialtion() {
    start_servo_vent();
    start_servo();
    start_vent(true);
}
void end_ventialtion() {
    end_servo();
    start_vent(false);
    end_servo_vent();
}

//MQTT
void onConnectionEstablished()
{
  // wild card subscription
 client.subscribe("aau_gh/control/water_pump", [](const String & topic, const String & payload)
  { 
   if (payload.equals("true"))
   {
    start_water_pump(true);
   } else
   {
    start_water_pump(false);
   }
   
   
   // more omplementation. For example, get the topic adn payload value and perform a specific operation
   // -- more implementation here -- 
  });  
 
 client.subscribe("aau_gh/control/fan", [](const String & topic, const String & payload)
  { 
   if (payload.equals("true"))
   {
    start_ventialtion();
   } else
   {
    end_ventialtion();
   }
   
   // more omplementation. For example, get the topic adn payload value and perform a specific operation
   // -- more implementation here -- 
  }); 

client.subscribe("aau_gh/control/automatic", [](const String & topic, const String & payload)
  { 
   if (payload.equals("true"))
   {
    automatic = true;
   } else
   {
    automatic = false;
   }
   
   // more omplementation. For example, get the topic adn payload value and perform a specific operation
   // -- more implementation here -- 
  });  

}

// Structior with all values
struct state
{
    int rainsensor;
    int moisture;
    float temp;
    float pressure;
    int light;
    int uv;
    float soil_temp;
};

void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pinMode(LED_PIN_green,  OUTPUT);
  pinMode(LED_PIN_blue,  OUTPUT);
  pinMode(VENT_PIN, OUTPUT);
  pinMode(WATER_PIN,OUTPUT);
  pinMode(SERVO_PIN,OUTPUT);
  pinMode(rainpin, INPUT);

  // Optionnal functionnalities of EspMQTTClient :
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  // client.enableHTTPWebUpdater(); // Enable the web updater.

  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Ich verbinde mich mit dem Internet...");
  }
  Serial.println("Ich bin mit dem Internet verbunden!");
}

void loop() {
 
 client.loop();

 //Struction
  state GreenhouseValues;
  
  // give some break
  if((millis() - last_time) > time_elapsed)
  {
    GreenhouseValues.rainsensor = getRainSensor();           
    GreenhouseValues.moisture   = getSoilMoistureSensor();  
    GreenhouseValues.temp       = getBMP280_Temperatur(); 
    GreenhouseValues.pressure   = getBMP280_Pressure();   
    GreenhouseValues.light      = getTS2591();    
    GreenhouseValues.uv         = getUVM30A();
    GreenhouseValues.soil_temp  = getSoil_Temperatur();  
    
    // publish sensor values as message to the broker
    client.publish("aau_gh/climate/temperature", (String)GreenhouseValues.temp);
    client.publish("aau_gh/climate/soil_temp", (String)GreenhouseValues.soil_temp);
    client.publish("aau_gh/climate/moisture", (String)GreenhouseValues.moisture);
    client.publish("aau_gh/climate/rain", (String)GreenhouseValues.rainsensor);
    client.publish("aau_gh/climate/pressure", (String)GreenhouseValues.pressure);
    client.publish("aau_gh/climate/light", (String)GreenhouseValues.light);
    client.publish("aau_gh/climate/uv-light", (String)GreenhouseValues.uv); 
    Serial.println(automatic);
    Serial.println(count);
    Serial.println();
    
    print_display_data();

    if (count_check)
    {
      count++;
    }
    
    // reset last seen
    last_time = millis();
  } 

  client.loop();

  if ((GreenhouseValues.rainsensor >= 20) && (rain_check != true) && (automatic == true))
  {
    end_ventialtion();
    rain_check = true;
  }
  
  if (GreenhouseValues.rainsensor <= 10)
  {
    rain_check = false;
  }
  

//  ----------------- //

  // AUTOMATIC FAN

  if ((automatic==false)&&(check1==true))
  {
    end_ventialtion();
    check1 = false;
    vent_count = true;
  }
   

  if ( (automatic == true) && (GreenhouseValues.rainsensor <= 20) && (GreenhouseValues.temp >= (float)24.0))
  {
    if (vent_count)
    {
      start_ventialtion();
      check1 = true;
    }
    
    vent_count = false;
  }

  if ((automatic == true) && (GreenhouseValues.temp<=(float)23.0) && (check1 == true))
  {
    end_ventialtion();
    check1 = false;
    vent_count = true;
  }
  
  // AUTOMATIC WATER PUMP

//  --------------------------- //


  if ((automatic == true) && (GreenhouseValues.moisture <= 30) && (count < 10) && (check2 == false))
  {
    start_water_pump(true);
    check2 = true;
    count_check = true;
  }
  if ((automatic == true) && (GreenhouseValues.moisture >=30))
  {
    start_water_pump(false);
    check2 = false;
  }

   if ((automatic == false) && (check2 == true))
  {
    start_water_pump(false);
    check2 = false;
    count = 0;
    count_check = false;
  }

  if ((count >= 10) && (automatic == true))
  {
    start_water_pump(false);
    check2 = false;
  }

  if (count == 120)
  {
    count_check = false;
    count = 0;
  }
  

  // AUTOMATIC RGB-LED

//  --------------------------- //

  if ((GreenhouseValues.light <= 5))
  {
    RGB_LED_blue(true);
    check3 = true;
  }

   if ((GreenhouseValues.light >= 5))
  {
    RGB_LED_blue(false);
    check3 = false;
  }

}