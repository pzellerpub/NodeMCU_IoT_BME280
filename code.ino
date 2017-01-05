/******************************************************************************************
 * IoT Environmental Data NodeMCU, Bosch BME280 via I2C, LCD 1602 via I2C Frentaly® IIC/I2C/TWI 1602 
 * 
 * Code for implementation of BME280 is based on the original samples by
 * Marshall Taylor @ SparkFun Electronics
 * May 20, 2015
 * https://github.com/sparkfun/SparkFun_BME280_Arduino_Library
 * 
 * Adaption and implementaion by Patrick Zeller
 * 
 * Resources: 
 * Wire.h for I2C operation - used with NodeMCU / ESP8266 aditional parameter for I/O are required
 * ESP8266WiFi.h for WiFi connection.
 * 
 * IDE: Arduino IDE 1.8.0
 * 
 * This code is released under the [MIT License](http://opensource.org/licenses/MIT).
 ******************************************************************************************/
#include <ESP8266WiFi.h>          // For Wifi
#include "SparkFunBME280.h"       // This was the only BME280 Library working for me, even if I do have the Adafruit module and not the SparkFun one.
#include <stdint.h>
#include "Wire.h"                 // Library required for I2C connection
#include <LiquidCrystal_I2C.h>    // Library found here: https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads - I had to use Version 1.2.1 from 2012, >>NOT<< the NEWliquidCrystal one. I wasted hours until I figured that out.

const char* ssid = "YOURSSID";  // Enter your WiFi Name, SSID
const char* password = "YOURWIFI-PW"; //Your WiFi PW
int lcdConnectWlan = 0; // Optional usage, just for design to delay the connected msg 
int countsenddata = 27; // Used to count the 1Hz runs of live data before the upload is initalized 
String ApiKey = "O33CVV4JCT75ZD8A";  // Enter your Thingspeak WRITE Api here
String path = "/update?key=" + ApiKey;  //Path for Thingspeak
const char* host = "api.thingspeak.com"; //Host for Thingspeak
const int httpPort = 80;  //Port for Thingspeak
int tempdatasend; //I was lazy by using help variables instead of calling the proper function 
int humiditydata; //I was lazy by using help variables instead of calling the proper function 
int pressuredata; //I was lazy by using help variables instead of calling the proper function 
int usrnamelcd = "NAME" //Enter the name shown during the display initializes. 

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address, in this case 0x3F - If you don`t know it, use the I2C scanning script, don`t forget to set I/O Pins after Wire.begin (x, y);
BME280 mySensor; //Global sensor object

void setup()
{
  Serial.begin(115200);   //Setup Serial Com
  Wire.begin(4, 5);       //Setup I2C connection on GPIO 4 and 5, which is D2 and D1 at the NodeMCU. GPIO5 (D1) is SCL and GPIO4 (D2) is SDA
  delay(20);              //20ms delay to intialize
     lcd.begin(16,2);     //initialize the display, 16 chars @ 2 rows. 
     for(int i = 0; i< 3; i++) //option for-loop to blink the display at the beginning. 
  {
    lcd.backlight();
    delay(500);
    lcd.noBacklight();
    delay(500);
  }
  lcd.backlight(); // finish for-loop with backlight on  

   Serial.println();

  lcd.setCursor(0,0); //Start at character 0 on line 0
  lcd.print("Hello " + usrnamelcd + "!");  //Welcome Msg
  delay(1500);
  lcd.clear();  //clear the LCD
  Serial.println();
                                
  Serial.print("Connecting to ");     //Connection MSg
  lcd.setCursor(0,0);           //set cursor to character 0 on line 0
  lcd.print("WLAN: ");
  lcd.print(ssid);
  Serial.println(ssid);
                                
  WiFi.begin(ssid, password);   //Connect to Wifi with SSID and PASSWORD
  
for (int i = 0; i< 2; i++)      //Optional for-loop to blink twice the "connecting" msg. The NodeMCU / ESP8266 connects so fast that most of the time it jumped right to the connected msg without this loop. It just looks nicer....
{
      lcd.setCursor(0,1);
      lcd.print("   connecting   ");
      delay(500);
      lcd.setCursor(0,1);
      lcd.print("                ");        
      delay(500);
}


 
  while (WiFi.status() != WL_CONNECTED) {     //blink the "connecting" MSG unless the connection is established. If your SSID and PW are correct, you won`t see this most of the time.
                                              //I had sometimes connection issues for several hours, figured out that it was either my wireing or power supply
    
    Serial.print(".");
    if (lcdConnectWlan < 1)
    {
      lcd.setCursor(0,1);
      lcd.print("   connecting   ");
      lcdConnectWlan = 1;
      }
      else
      {
      lcd.setCursor(0,1);
      lcd.print("                ");        
      lcdConnectWlan = 0;
      }
    delay(500);
  }
  lcd.setCursor(0,1);
  Serial.println("");
  Serial.println("WiFi connected");         //Print the connected msg after success
  lcd.print("!! Connected !!");
  delay(2000);
 
                                            // Print the IP address
  Serial.print("Your IP is: ");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("My IP is:");
  lcd.setCursor(0,1);
  lcd.print(WiFi.localIP());
  delay(4000);

  
	//***Driver settings********************************//
	//specify chipSelectPin using arduino pin names
	//specify I2C address.  Can be 0x77(default) or 0x76	      -- Comment by Patrick:My BME280 uses 0x77, my BMP280 is default at 0x76 - If you are not sure, use the I2C scan script, google it!
	mySensor.settings.commInterface = I2C_MODE;
	mySensor.settings.I2CAddress = 0x77;
	//***Operation settings*****************************//
	//renMode can be:
	//  0, Sleep mode
	//  1 or 2, Forced mode
	//  3, Normal mode
	mySensor.settings.runMode = 3; //Normal mode
		//tStandby can be:
	//  0, 0.5ms
	//  1, 62.5ms
	//  2, 125ms
	//  3, 250ms
	//  4, 500ms
	//  5, 1000ms
	//  6, 10ms
	//  7, 20ms
	mySensor.settings.tStandby = 2;
		//filter can be off or number of FIR coefficients to use:
	//  0, filter off
	//  1, coefficients = 2
	//  2, coefficients = 4
	//  3, coefficients = 8
	//  4, coefficients = 16
	mySensor.settings.filter = 0;
		//tempOverSample can be:
	//  0, skipped
	//  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
	mySensor.settings.tempOverSample = 1;
	//pressOverSample can be:
	//  0, skipped
	//  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
    mySensor.settings.pressOverSample = 1;
	//humidOverSample can be:
	//  0, skipped
	//  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
	mySensor.settings.humidOverSample = 1;
	
	Serial.print("Program Started\n");
	Serial.print("Starting BME280... result of .begin(): 0x");
	
	//Calling .begin() causes the settings to be loaded
	//delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
	Serial.println(mySensor.begin(), HEX);

	Serial.print("Displaying ID, reset and ctrl regs\n");
	
	Serial.print("ID(0xD0): 0x");
	Serial.println(mySensor.readRegister(BME280_CHIP_ID_REG), HEX);
	Serial.print("Reset register(0xE0): 0x");
	Serial.println(mySensor.readRegister(BME280_RST_REG), HEX);
	Serial.print("ctrl_meas(0xF4): 0x");
	Serial.println(mySensor.readRegister(BME280_CTRL_MEAS_REG), HEX);
	Serial.print("ctrl_hum(0xF2): 0x");
	Serial.println(mySensor.readRegister(BME280_CTRL_HUMIDITY_REG), HEX);

	Serial.print("\n\n");

	Serial.print("Displaying all regs\n");
	uint8_t memCounter = 0x80;
	uint8_t tempReadData;
	for(int rowi = 8; rowi < 16; rowi++ )
	{
		Serial.print("0x");
		Serial.print(rowi, HEX);
		Serial.print("0:");
		for(int coli = 0; coli < 16; coli++ )
		{
			tempReadData = mySensor.readRegister(memCounter);
			Serial.print((tempReadData >> 4) & 0x0F, HEX);//Print first hex nibble
			Serial.print(tempReadData & 0x0F, HEX);//Print second hex nibble
			Serial.print(" ");
			memCounter++;
		}
		Serial.print("\n");
	}
	
	
	Serial.print("\n\n");
	
	Serial.print("Displaying concatenated calibration words\n");
	Serial.print("dig_T1, uint16: ");
	Serial.println(mySensor.calibration.dig_T1);
	Serial.print("dig_T2, int16: ");
	Serial.println(mySensor.calibration.dig_T2);
	Serial.print("dig_T3, int16: ");
	Serial.println(mySensor.calibration.dig_T3);
	
	Serial.print("dig_P1, uint16: ");
	Serial.println(mySensor.calibration.dig_P1);
	Serial.print("dig_P2, int16: ");
	Serial.println(mySensor.calibration.dig_P2);
	Serial.print("dig_P3, int16: ");
	Serial.println(mySensor.calibration.dig_P3);
	Serial.print("dig_P4, int16: ");
	Serial.println(mySensor.calibration.dig_P4);
	Serial.print("dig_P5, int16: ");
	Serial.println(mySensor.calibration.dig_P5);
	Serial.print("dig_P6, int16: ");
	Serial.println(mySensor.calibration.dig_P6);
	Serial.print("dig_P7, int16: ");
	Serial.println(mySensor.calibration.dig_P7);
	Serial.print("dig_P8, int16: ");
	Serial.println(mySensor.calibration.dig_P8);
	Serial.print("dig_P9, int16: ");
	Serial.println(mySensor.calibration.dig_P9);
	
	Serial.print("dig_H1, uint8: ");
	Serial.println(mySensor.calibration.dig_H1);
	Serial.print("dig_H2, int16: ");
	Serial.println(mySensor.calibration.dig_H2);
	Serial.print("dig_H3, uint8: ");
	Serial.println(mySensor.calibration.dig_H3);
	Serial.print("dig_H4, int16: ");
	Serial.println(mySensor.calibration.dig_H4);
	Serial.print("dig_H5, int16: ");
	Serial.println(mySensor.calibration.dig_H5);
	Serial.print("dig_H6, uint8: ");
	Serial.println(mySensor.calibration.dig_H6);
  Serial.println();
  lcd.clear();
}

void loop()
{

 if (countsenddata < 25)                                //The script runs 25 times = 25 secounds until the upload starts. with uploads and related delays this results in one upload every 30sec
 {

                                                        //The script provides a hell of date and calculations. I print all on the serial port, but use only Temp Celcius, Humidity % and Pressure PA for the LCD and Thingspeak.
	Serial.print("Temperature: ");
	Serial.print(mySensor.readTempC(), 2);
	Serial.println(" degrees C");
  lcd.setCursor(0,0);
  lcd.print(mySensor.readTempC(),2);
  lcd.print("*C | ");
  lcd.print(mySensor.readFloatHumidity(),2);
  lcd.print("%");
  lcd.setCursor(0,1);
  lcd.print(mySensor.readFloatPressure(),2);
  lcd.print(" Pa");
  Serial.print("Temperature: ");
	Serial.print(mySensor.readTempF(), 2);
	Serial.println(" degrees F");
  Serial.print("Pressure: ");
  Serial.print(mySensor.readFloatPressure(), 2);
  Serial.println(" Pa");
	Serial.print("Altitude: ");
	Serial.print(mySensor.readFloatAltitudeMeters(), 2);
	Serial.println("m");
	Serial.print("Altitude: ");
	Serial.print(mySensor.readFloatAltitudeFeet(), 2);
	Serial.println("ft");	
	Serial.print("%RH: ");
	Serial.print(mySensor.readFloatHumidity(), 2);
	Serial.println(" %");
	Serial.println();
  delay(1000);	
  countsenddata = countsenddata + 1;
 }
else
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Uploading data...");                 //MSG are only for user interaction and are optional.
  lcd.setCursor(0,1);
  lcd.print("Connecting...");
  delay(2000);
  WiFiClient client;                              //Setup Wifi client socket
  if (!client.connect(host, httpPort)) {          //Connecting to host and port - Thingspeak
    Serial.println("connection failed");          //Error handling
    lcd.setCursor(0,1);
    lcd.print("Connection failed!");               //Error Msg
    delay(2000);
    return;
  }
tempdatasend = mySensor.readTempC(), 2;            
humiditydata = mySensor.readFloatHumidity(), 2;
pressuredata = mySensor.readFloatPressure(), 2;
client.print(String("GET http://api.thingspeak.com/update?api_key=") + ApiKey + "&field1=" + tempdatasend + "&field2=" + humiditydata + "&field3=" + pressuredata + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" +  String("Connection: keep-alive\r\n\r\n"));   //The GET spell to update related fields at the your Thingspeak channel
    lcd.setCursor(0,1);
    lcd.print("Upload Done!");        //Upload done Msg
    Serial.print("Upload Done!");
  delay(2000);
  lcd.clear();
  countsenddata = 0;                  //Reset the count before data is newly send.
  }
  

}
