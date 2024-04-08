#include <DHT.h>    //libreria per sensore di umidità e temperatura DHT11
#include <SPI.h>    //libreria per scheda SD
#include <SD.h>     //libreria per scheda SD
#include <Wire.h>   //libreria per bmp280

#include <Adafruit_Sensor.h>    //libreria per sensori Adafruit
#include <Adafruit_BMP280.h>    //libreria per sensore di pressione bmp280

Adafruit_BMP280 bmp;            // I2C Interface

#define DHTPIN 8          // 8 è il pin di Arduino a cui collego il sensore di temperatura/umidità DHT11
#define DHTTYPE DHT11     // dht11 è il tipo di sensore che uso
DHT dht(DHTPIN, DHTTYPE);
unsigned long time;
float QNH=970.75;   //insert the value of the pressure at the sea level (hPa) in your city in order to calculate the altitude
                //you can also insert the pressure of a starting point and use this function as an altimeter

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
// MKRZero SD: SDCARD_SS_PIN
const int chipSelect = 10;
int t_start = millis;

void setup() {
  pinMode(8,INPUT);     //pin 8 utilizzato per DHT11, umidità e temperatura (dichiarato di tipo INPUT)
  delay(2000);
  Serial.begin(9600);

/* Test sul corretto collegamento del sensore di pressione bmp280 */


  Serial.println(F("BMP280 test"));

  if (!bmp.begin()) {     
    Serial.println(bmp.begin());
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

/* Test sulla presenza di scheda SD con formattazione corretta */

while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print(F("\nInitializing SD card..."));

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println(F("initialization failed. Things to check:"));
    Serial.println(F("* is a card inserted?"));
    Serial.println(F("* is your wiring correct?"));
    Serial.println(F("* did you change the chipSelect pin to match your shield or module?"));
    Serial.println(F("* card not found, continue..."));
  //  while (1);
  } else {
    Serial.println(F("Wiring is correct and a card is present."));
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println(F("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card"));
  //  while (1);
  }

  if (!SD.begin(chipSelect)) {
      Serial.println(F("Errore SD"));
      return;
    }
  
  Serial.println(F("SD pronta"));

  if (SD.exists("meteo.csv")) {            //cancella il file se ne trova uno con lo stesso nome
     SD.remove("meteo.csv");
     Serial.println(F("File SD/meteo.csv presente, viene cancellato"));
    } else {
    Serial.println(F("File non trovato"));
    }

  //Scrivo l'header del file meteo.csv

  File file = SD.open("meteo.csv", FILE_WRITE);   
  if (file)
    {
      String testata_file = "Umid,TempDHT,Press,Tempbmp,Altitud,TempMedia,Dt,Tempo";  //These will be the headers for your excel file, CHANGE "" to whatevr headers you would like to use
      file.println(testata_file);
      file.close();
      Serial.println(F("File SD/meteo.csv creato correttamente"));
      Serial.println();
      Serial.println(testata_file);
    }

}
void loop() {
//Serial.flush();

/* Lettura di temperatura e umidità dal sensore DHT11 */

  int t_dht = dht.readTemperature();
  int h = dht.readHumidity();
  //lettura_sensore=analogRead(pin8);
  
  Serial.print(F("\nTemperatura_DHT = "));
  Serial.print(t_dht);
  Serial.print(F(" *C"));
  Serial.print(F("\nUmidita' = "));
  Serial.print(h);
  Serial.print(F(" %"));

/* Lettura di pressione e temperatura dal sensore bmp280 */

  float t_bmp = bmp.readTemperature();
  float p = bmp.readPressure()/100;
  float alt = bmp.readAltitude(QNH);
  float t_mean = (t_dht + t_bmp)/2;

    Serial.print(F("\nTemperature_BMP = "));
    Serial.print(t_bmp);
    Serial.print(F(" *C"));

    Serial.print(F("\nPressure = "));
    Serial.print(p); //displaying the Pressure in hPa, you can change the unit
    Serial.print(F(" hPa"));

    Serial.print(F("\nApprox altitude = "));
    Serial.print(alt); //The "1019.66" is the pressure(hPa) at sea level in day in your region
    Serial.print(F(" m"));                    //If you don't know it, modify it until you get your current altitude

    Serial.print(F("\nTemp Media = "));
    Serial.print(t_mean); //The "1019.66" is the pressure(hPa) at sea level in day in your region
    Serial.print(F(" °C"));                    //If you don't know it, modify it until you get your current altitude


    /* Metodo alternativo per calcolare l'altitudine, basato sulle formule dell'atm strandrad */
    //float alt;
    //alt=(288.16/0.0065)*(1-(p/QNH)^(1/5.25));
    //Serial.println();
    //Serial.println(alt);

    Serial.println();

//Time
  int time = millis();
  int Dt = (millis() - t_start)/1000;

//Scrivo i valori nel file meteo.csv
File file = SD.open("meteo.csv", FILE_WRITE);
String parametri = String(h) + "," + String(t_dht) + "," + String(p) + "," + String(t_bmp) + "," + String(alt) + "," + String(t_mean) + "," + String(Dt) + "," + String(time);
    
  if (parametri)
  {
    file.println(parametri);
    file.close();
    Serial.println(parametri);
  }

  delay(1000);

}