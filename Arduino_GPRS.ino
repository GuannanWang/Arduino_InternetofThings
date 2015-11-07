/*
 Arduino-Sim900_Internet
 By Guannan Wang 2015-10-25
 
 Basic sketch for GPRS shield sim900
 This code is based on the work done by 
 Mateo Velez - Metavix - for Ubidots Inc. (2014-08-20), as well as
 Jeffery Sadler https://github.com/jsadler2/stormwater/blob/master/raingage.ino
 
 This is a basic example to post a value on Ubidots with a simple
 function "save_value".

 You'll need:
 * An Arduino Uno
 * A GPRS shield Sim900

 This example code is in the public domain.
*/

//--------------------------------------------------------------
//------------------------------Libraries-----------------------
//--------------------------------------------------------------
#include <SoftwareSerial.h>
#include <String.h>
#include "DHT.h"
#define NOT_AN_INTERRUPT -1
SoftwareSerial mySerial(7, 8);                                                      //your pins to serial communication, for some Sim900 board, they are (2,3)
int valor;

// for DHTTYPE
#define DHTTYPE DHT22   // DHT 22  (AM2302)

int sensorPin = A0;
int reading;
const int rainGauge =3;
volatile int rainCount = 0;
float rainCorrect;

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(sensorPin, DHTTYPE);

//-------------------------------------------------------------
//---------------------Ubidots Configuration-------------------
//-------------------------------------------------------------
String token = "FsgQDF7vkDBA750zxhe8QCPJHuAlkYxjLGNi47sBiMzaQzsIHr020qtADKjC";      //your token to post value
String idvariableT = "562d523c762542758a2b8382";     //ID of your temperature
String idvariableH = "562d61ed7625422d332ce386";     //ID of your humidity
String idvariableR = "563195ed7625423a9e51b0eb";     //ID of your rain fall
void setup()
{
  mySerial.begin(19200);                                                            //the GPRS baud rate
  Serial.begin(19200);     //the serial communication baud rate
  
  analogReference(DEFAULT);
  attachInterrupt(digitalPinToInterrupt(rainGauge),rainGageClick,RISING);           // Digital Pins with Interrupts 
                                                                                    // https://www.arduino.cc/en/Reference/AttachInterrupt
  Serial.println(rainGauge);

  delay(10000);
}

void loop()
{
    int value = analogRead(A0);
//
    int sensorReadingT = readDHTSensorT();
    int sensorReadingH = readDHTSensorH();
    Serial.println("sensorReadingT");
    Serial.println(sensorReadingT);
    Serial.println("sensorReadingH");
    Serial.println(sensorReadingH);
// rain gauge    
    rainCorrect= rainCount/100.0;
    Serial.println(rainCorrect);
    
    //read pin A0 from your arduino
    save_value(String(sensorReadingT),idvariableT);   //call the save_value function
    save_value(String(sensorReadingH),idvariableH);   
    save_value(String(rainCorrect),idvariableR);      
   
    if (mySerial.available())
    Serial.write(mySerial.read());
}
//this function is to send the sensor data to Ubidots, you should see the new value in Ubidots after executing this function
void save_value(String value, String idvariable)
{
  int num;
  String le;
  String var;
  var="{\"value\":"+ value + "}";
  num=var.length();
  le=String(num);
  for(int i = 0;i<3;i++)
  {
    mySerial.println("AT+CGATT?");                                                   //this is made repeatedly because it is unstable
    delay(2000);
    ShowSerialData();
  }
  mySerial.println("AT+CSTT=\"CMNET\",\"\",\"\"");                                   //replace with your providers' APN
  delay(1000);
  ShowSerialData();
  mySerial.println("AT+CIICR");                                                      //bring up wireless connection
  delay(3000);
  ShowSerialData();
  mySerial.println("AT+CIFSR");                                                      //get local IP adress
  delay(2000);
  ShowSerialData();
  mySerial.println("AT+CIPSPRT=0");
  delay(3000);
  ShowSerialData();
  mySerial.println("AT+CIPSTART=\"tcp\",\"things.ubidots.com\",\"80\"");             //start up the connection
  delay(3000);
  ShowSerialData();
  mySerial.println("AT+CIPSEND");                                                    //begin send data to remote server
  delay(3000);
  ShowSerialData();
  mySerial.print("POST /api/v1.6/variables/"+idvariable);
  delay(100);
  ShowSerialData();
  mySerial.println("/values HTTP/1.1");
  delay(100);
  ShowSerialData();
  mySerial.println("Content-Type: application/json");
  delay(100);
  ShowSerialData();
  mySerial.println("Content-Length: "+le);
  delay(100);
  ShowSerialData();
  mySerial.print("X-Auth-Token: ");
  delay(100);
  ShowSerialData();
  mySerial.println(token);
  delay(100);
  ShowSerialData();
  mySerial.println("Host: things.ubidots.com");
  delay(100);
  ShowSerialData();
  mySerial.println();
  delay(100);
  ShowSerialData();
  mySerial.println(var);
  delay(100);
  ShowSerialData();
  mySerial.println();
  delay(100);
  ShowSerialData();
  mySerial.println((char)26);
  delay(7000);
  mySerial.println();
  ShowSerialData();
  mySerial.println("AT+CIPCLOSE");                                                //close the communication
  delay(1000);
  ShowSerialData();
}

void ShowSerialData()
{
  while(mySerial.available()!=0)
  Serial.write(mySerial.read());
}

void rainGageClick(){
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (interrupt_time - last_interrupt_time > 200) 
 {
   rainCount++;
 }
 last_interrupt_time = interrupt_time;
}


// get data from the DHT22
int readDHTSensorT(){
   int reading = analogRead(sensorPin);  
 
   // converting that reading to voltage, for 3.3v arduino use 3.3
   float h = dht.readHumidity();
   int t = dht.readTemperature();  
   // print out the voltage
   Serial.print("Humidity:"); 
   Serial.println(h);      //to degrees ((voltage - 500mV) times 100)
   Serial.print("Temperature:"); 
   Serial.print(t);
   Serial.println(" degree(C)");   
   return t;
   Serial.println(t);
}

// get data (humidity) from the DHT22
int readDHTSensorH(){
   int reading = analogRead(sensorPin);  
 
   // converting that reading to voltage, for 3.3v arduino use 3.3
   float h = dht.readHumidity();
   int t = dht.readTemperature();  
   // print out the voltage
   Serial.print("Humidity:"); 
   Serial.println(h);     
   Serial.print("Temperature:"); 
   Serial.print(t);
   Serial.println(" degree(C)");   
   return h;
   Serial.println(h);
}
