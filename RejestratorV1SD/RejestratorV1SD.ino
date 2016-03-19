/*
Rejestrator temperatury
 */



#include <SPI.h>
#include <SD.h>
#include <OneWire.h>



//***SD CARD***
  // set up variables using the SD utility library functions:
  Sd2Card card;
  SdVolume volume;
  SdFile root;  
// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
const int chipSelect = 4;

//DS18B20
OneWire  ds(7);  // on pin 10 (a 4.7K resistor is necessary)
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  float temperatura;


//Zmienne:
 float anaglogPinVoltage = 44.420;
 double i ;
 File myFile;
 unsigned long czasDzialania;

float przeliczZakresaNaVolty (int wartoscAnalogowa )
{
anaglogPinVoltage = wartoscAnalogowa * 0.0048875855327468;
return anaglogPinVoltage;
  
}
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  testSD();
  testDS18B20();
  SD.begin(4);
}


void loop() {
  i++;
  //Odczyt i konwersja napiecia na A0:
  /*int sensorReading = analogRead(A0);
  Serial.print("Sensor reading:");
  Serial.println(sensorReading);
  anaglogPinVoltage = przeliczZakresaNaVolty(sensorReading);
   Serial.print("V A0: ");
   Serial.print(anaglogPinVoltage);
   Serial.println(" V. ");*/
  //Odczytanie czasu
  czasDzialania= millis()/1000;

  // Odczytanie temepratury
  temperatura = zmierzTemperatureDS18B20();
  // zmierzTemperatureDS18B20();
   //Zapis na kare SD:
     File myFile = SD.open("data.txt", FILE_WRITE);
   //myFile = SD.open("data.txt", FILE_WRITE);
   if (myFile) 
   {
     myFile.print(temperatura);
     myFile.print(",");
     myFile.print(czasDzialania);
     myFile.print(",");
     myFile.print(i);
     myFile.println("");
     myFile.close();
    
   }
   else 
   {
    Serial.println("error opening data.txt");
  }
    testSD();
    Serial.print("Temperatura = ");
    Serial.print(celsius);
     Serial.println(" C ");
    Serial.print("Czas dzialania urzadzenia: ");
    Serial.print(czasDzialania);
    Serial.println(" sekund.");
    Serial.println("--------------------------------");
   
 
   
   delay(30000);
} 
  

  void testSD()
{
  //Serial.print("\nInitializing SD card...");

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
    //Serial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  /*Serial.print("\nCard type: ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }*/

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }


  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  //Serial.print("\nVolume type is FAT");
 // Serial.println(volume.fatType(), DEC);
 // Serial.println();

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
 // Serial.print("Volume size (bytes): ");
 // Serial.println(volumesize);
//  Serial.print("Volume size (Kbytes): ");
  volumesize /= 1024;
 // Serial.println(volumesize);
 // Serial.print("Volume size (Mbytes): ");
  volumesize /= 1024;
 // Serial.println(volumesize);


 // Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_SIZE);

  
}

void testDS18B20()
{
  
  byte i ;
   if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 
  
}
float zmierzTemperatureDS18B20()
{
  byte i;
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

 /* Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  */
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    //Serial.print(data[i], HEX);
   // Serial.print(" ");
  }
  /*
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();*/

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  //fahrenheit = celsius * 1.8 + 32.0;
  
  return celsius;
 // Serial.print(fahrenheit);
 // Serial.println(" Fahrenheit");
}

