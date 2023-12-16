// Program RFIDStartRead
  // memulai mode pembacaan RFID scanner

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels

// create an OLED display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define rx2 16
#define tx2 17
#define button GPIO_NUM_13
#define led GPIO_NUM_32

#define DEBOUNCE_DURATION 300
#define READ_MODE 0
#define STOP_MODE 1

//COMMAND
//command untuk membaca device address
const byte cmd_readDevice[] = { 0xAA, 0xAA, 0xFF, 0x06, 0x05, 0x01, 0xFF, 0x1B, 0xA2 };
//command untuk setting frequency region 920.125~924.875MHz
const byte cmd_setFreqRegion[] = { 0xAA, 0xAA, 0xFF, 0x06, 0x30, 0x00, 0x01, 0x08, 0x17 };
//command untuk setting work frequency channel
const byte cmd_setFreqCh[] = { 0xAA, 0xAA, 0xFF, 0x06, 0x32, 0x00, 0x00, 0x76, 0x56 };
//command untuk turn Automatic Frequency Hopping Mode on
const byte cmd_autoFreqHop[] = { 0xAA, 0xAA, 0xFF, 0x06, 0x37, 0x00, 0xFF, 0x83, 0x56 };
//command untuk Set RF Emission Power Capacity 0BB8 = 3000 = 30 dBm
//byte cmd_PowEmmCap[] = {0xAA,0xAA,0xFF,0x07,0x3B,0x00,0x0B,0xB8,0xEB,0x5E};
//command untuk Set RF Emission Power Capacity 14 dBm
//byte cmd_PowEmmCap[] = { 0xAA, 0xAA, 0xFF, 0x07, 0x3B, 0x00, 0x05, 0x78, 0x11, 0x1D };
//command untuk Set RF Emission Power Capacity 20 dBm
//byte cmd_PowEmmCap[] = {0xAA,0xAA,0xFF,0x07,0x3B,0x00,0x07,0xD0,0x43,0x9D};
//command untuk Set RF Emission Power Capacity 30 dBm
const byte cmd_PowEmmCap[] = { 0xAA, 0xAA, 0xFF, 0x07, 0x3B, 0x00, 0x0B, 0xB8, 0xEB, 0x5E };
// command untuk single tag inventory
const byte cmd_startSingleRead[] = {0xAA,0xAA,0xFF,0x05,0xC8,0x00,0x3A,0x5E};
// command untuk set working parameters of the antenna
// jumlah antenna 1, port 1, polling open, power 30 dBm, batas kali bacaan per antenna 10
//byte cmd_setWorkParam[] = {0xAA, 0xAA, 0xFF, 0x0E, 0x3F, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xB8, 0x0A, 0x21, 0xB2};
// jumlah antenna 1, port 1, polling open, power 10 dBm, batas kali bacaan per antenna 10
//byte cmd_setWorkParam[] = {0xAA, 0xAA, 0xFF, 0x0E, 0x3F, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0xE8, 0x0A, 0x86, 0xAC};
// jumlah antenna 1, port 1, polling open, power 14 dBm, batas kali bacaan per antenna 10
//byte cmd_setWorkParam[] = {0xAA, 0xAA, 0xFF, 0x0F, 0x3F, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x05, 0x78, 0x00, 0x0A, 0x09, 0x0C};
// jumlah antenna 1, port 1, polling open, power 20 dBm, batas kali bacaan per antenna 200
//byte cmd_setWorkParam[] = {0xAA, 0xAA, 0xFF, 0x0F, 0x3F, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x07, 0xD0, 0x00, 0xC8, 0x09, 0x57};
// jumlah antenna 1, port 1, polling open, power 30 dBm, batas kali bacaan per antenna 200
const byte cmd_setWorkParam[] = { 0xAA, 0xAA, 0xFF, 0x0F, 0x3F, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xB8, 0x00, 0xC8, 0x74, 0xAF };
// command untuk start multiple tags in inventory
// mulai multiple read
const byte cmd_startRead[] = { 0xAA, 0xAA, 0xFF, 0x08, 0xC1, 0x00, 0x08, 0x00, 0x00, 0x60, 0x4A };
//menghentikan multiple read
const byte cmd_stopRead[] = { 0xAA, 0xAA, 0xFF, 0x05, 0xC0, 0x00, 0xB3, 0xF7 };

// inisiasi variabel
int mode = STOP_MODE;
int countButtonPressed = 0;
volatile bool gpio_intr_flag = false;

void IRAM_ATTR isr()
{
  gpio_intr_flag = true;
}

void setup()
{
  // konfigurasi komunikasi serial dari ESP32 ke Serial Monitor
  Serial.begin(115200);

  // konfigurasi komunikasi serial antara ESP32 dengan modul RFID
  Serial2.begin(115200, SERIAL_8N1, rx2, tx2);

  // Inisiasi RFID reader
  Serial2.write(cmd_readDevice, sizeof(cmd_readDevice));
  Serial2.write(cmd_setFreqRegion, sizeof(cmd_setFreqRegion));
  Serial2.write(cmd_setFreqCh, sizeof(cmd_setFreqCh));
  Serial2.write(cmd_autoFreqHop, sizeof(cmd_autoFreqHop));
  Serial2.write(cmd_PowEmmCap, sizeof(cmd_PowEmmCap));
  Serial2.write(cmd_setWorkParam, sizeof(cmd_setWorkParam));

  //setup interupsi tombol untuk ganti mode
  pinMode(button, INPUT);
  pinMode(led, OUTPUT);
  attachInterrupt(button, isr, RISING);

    // initialize OLED display with I2C address 0x3C
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
  }

  delay(2000);         // wait two seconds for initializing
  oled.clearDisplay(); // clear display
  oled.setTextSize(2);         // set text size
  oled.setTextColor(WHITE);    // set text color

  xTaskCreatePinnedToCore(buttonTask, "Button Task", 2048, NULL, 1, NULL, 0);
}

void buttonTask(void* pvParameters)
{
     (void)pvParameters;
  while (1) {
  if (mode == READ_MODE)
  {
    // kirim perintah untuk mulai baca tag
    // // single inventory
    // Serial2.write(cmd_startSingleRead, sizeof(cmd_startSingleRead));

    // multi inventory
    digitalWrite(led,HIGH);
    Serial2.write(cmd_startRead, sizeof(cmd_startRead));
  }
  else
  {
    // kirim perintah untuk stop baca tag
    digitalWrite(led,LOW);
    Serial2.write(cmd_stopRead, sizeof(cmd_stopRead));
  }
    vTaskDelay(1);
  }
}

void loop()
{
  if (true == gpio_intr_flag)
  {
    gpio_intr_flag = false;
    // debouncing
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();

    if (interrupt_time - last_interrupt_time > DEBOUNCE_DURATION)
    {
      // jika sedang mode baca dan belum selesai satu siklus, maka lanjut ke mode tampil
      Serial.print(countButtonPressed);
      countButtonPressed++;
      oled.clearDisplay();
      oled.setCursor(0, 10);       // set position to display
      if (mode == READ_MODE)
      {
        mode = STOP_MODE;
        Serial.println(". getting in READ_MODE");
        oled.println("Idle mode"); // set text
      }
      else
      {
        mode = READ_MODE;
        Serial.println(". getting in STOP_MODE");
        oled.println("Read Mode"); // set text
      }
    }
    oled.display();    
    last_interrupt_time = interrupt_time;
  }
  // put your main code here, to run repeatedly:
}
