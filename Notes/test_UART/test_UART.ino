// Program UARTTest
	// Membaca barcode scanner yang dihubungkan dengan Arduino nano dengan protokol komunikasi RS232
	// yang kemudian ditampilkan pada software serial
// KAMUS
// PINOUT
	// Arduino 	<-> Barcode
	// 5V		<-> 	5V
	// TX		<-> 	3
	// RX		<->		2
	// GND		<->		GND
// ALGORITMA UTAMA
#include<SoftwareSerial.h>
SoftwareSerial myserial(2,3);
String kode;

void setup() {
  Serial.begin(9600);
  
  myserial.begin(9600);
  myserial.setTimeout(100);
}

void loop() {
  while (myserial.available() > 0) {
    unsigned long startTime = millis();
    kode = myserial.readString();
    Serial.println(kode);
    unsigned long finishTime = millis();
    unsigned long duration = finishTime - startTime;
    Serial.println(duration);
  }