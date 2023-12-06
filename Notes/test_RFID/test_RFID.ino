// Program RFID
  // melakukan pembacaan RFID tag dan menampilkan pada serial monitor
  // Setiap button ditekan, maka satu kali siklus dijalankan
  // Saat LED hijau menyala, maka siap membaca RFID tag
  // Saat LED merah mati, maka RFID tag telah ditampilkan pada serial monitor
//pin komunikasi Serial2 untuk komunikasi dengan modul RFID
// KAMUS
  // Konstanta
    // maxTag := 100 { jumlah maksimum tag yang terbaca }
    // maxRespByte := 30
    // lenEPC := 12
    // tagLength := 24 { Panjang tag yang utuh}
  // Variabel
    // mode : integer {0 : baca data, 1 : tampilkan data}
    // tagValid : integer { menyatakan tag yang terbaca dan sudah dihitung CRC-nya dan siap untuk dikirim }
    // tag : integer { menyatakan tag yang terbaca }
    // flag_COMPLETE_ONE_CYCLE { menyatakan selesai satu siklus baca-tampil}
    // flag_COMPLETE_DISPLAY { menyatakan selesai tampil }
    // flag_COMPLETE_READ { menyatakan selesai baca }
  // Command
    // cmd_readDevice : baca alamat devais
    // cmd_setFreqRegion : atur region frekuensi
    // cmd_setFreqCh : atur channel frekuensi
    // cmd_autoFreqHop : automatic frequency hopping (ON)
    // cmd_PowEmmCap : atur kapasitas emisi daya 30 dBm
    // cmd_setWorkParam : jumlah antenna 1, port 1, polling open, power 30 dBm, batas kali bacaan per antenna 200
    // cmd_startRead : mulai baca
    // cmd_stopRead : berhenti baca
    /* cara kirim perintah ke RFID : 
          >>> Serial2.write(cmdx, sizeof(cmdx));   x : nomor perintah
    */
  // Fungsi/Prosedur
    // compareTags(tag1, tag2, start, end) -> bool
      // memeriksa duplikasi tag
    // Calculate_CRC(ptr, len) -> integer (unsigned)
      // menghitung CRC
    // procedure reinitVariabeles
      // inisiasi ulang variabel setelah satu siklus

// PINOUT
  //    ESP32     <->     x
  // ================================
  //    D25       <->   redLED
  //    D32       <->   greenLED
  //    D13       <->   button
  //    D16 (Rx)  <->   RFID Scanner (Tx)
  //    D17 (Tx)  <->   RFID Scanner (Rx)
// ALGORITMA UTAMA
#define rx2 16
#define tx2 17

// KONSTANTA
#define READ_TAG_MODE 0
#define DISPLAY_TAG_MODE 1
#define tagLength 24
#define DEBOUNCE_DURATION 300
const int maxTag = 100;
const int maxRespByte = 30;
const int lenEPC = 12;
const byte header[] = { 0xAA, 0xAA, 0xFF };

// VARIABEL
byte resp[maxTag][maxRespByte];
byte readyToSend[maxTag][lenEPC];
int tagValid = 0;
int mode = 0;
int tag = -1;
int flag_COMPLETE_ONE_CYCLE = 0;
int flag_COMPLETE_DISPLAY = 0;
int flag_COMPLETE_READ = 0;

//pin tombol dan LED
int button = 13;
int redLED = 25;
int greenLED = 32;

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
// byte cmd6[] = {0xAA,0xAA,0xFF,0x05,0xC8,0x00,0x3A,0x5E};
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
//reset reader
// byte cmd_readDevice0[] = { 0xAA, 0xAA, 0xFF, 0x05, 0x0F, 0x00, 0xB5, 0x9D };

// Interrupt external (button) untuk ganti mode
void IRAM_ATTR isr()
{
  // debouncing
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();

  if (interrupt_time - last_interrupt_time > DEBOUNCE_DURATION)
  {
    // jika sedang mode baca dan belum selesai satu siklus, maka lanjut ke mode tampil
    if ((mode == 0)&& (!flag_COMPLETE_ONE_CYCLE))
    {
      mode = 1;
    }
    // jika sedang dalam mode kirim
   else
   {
      mode = 0;

      // jika sedang dalam mode tampil dan button ditekan dan sudah selesai satu siklus
      if (flag_COMPLETE_ONE_CYCLE)
      {
        flag_COMPLETE_ONE_CYCLE = 0;
        flag_COMPLETE_READ = 0;
        Serial.println("tekan sekali lagi untuk ulangi siklus");
      }
    }
  }
  last_interrupt_time = interrupt_time;
}

/* Referensi tentang IRAM_ATTR
    >> https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/memory-types.html
*/

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
  pinMode(button, INPUT_PULLUP);
  attachInterrupt(button, isr, FALLING);

  //setup pin LED sebagai output
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  // kedua task di core yang berbeda / jalan secara parallel
  xTaskCreatePinnedToCore(mainTask, "Task1", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(ledTask, "Task2", 2048, NULL, 1, NULL, 0);
}

void mainTask(void* pvParameters)
{
  (void)pvParameters;
  while (1)
  {
    if (!flag_COMPLETE_READ)
    {
      Serial.println("--- AWAL SIKLUS ---");
      Serial.println("Membaca RFID tag... Tekan Push Button untuk tampil ke serial.");
    }

    if ((mode == READ_TAG_MODE) && (!flag_COMPLETE_ONE_CYCLE)) // mode baca RFID tag
    {  
      //baca multiple tag secara terus menerus
      int respByte = 0;
      Serial2.write(cmd_startRead, sizeof(cmd_startRead)); // mulai baca
     
      while (Serial2.available() > 0 && mode == READ_TAG_MODE)
      {
        int newTag = 0;
        // mencari respons modul RFID yang diawali dengan 0xAA, 0xAA, 0xFF
        if (Serial2.find(header, 3)) // syntax : Serial2.find(target, length)
        {
          byte incomingByte = Serial2.read();
          int lenData = int(incomingByte);
          // tag yang berhasil terbaca dengan sempurna memiliki panjang data 24 byte
          if (lenData == tagLength)
          {
            tag++; // inkremen indeks/jumlah tag yang terbaca
            newTag = 1; // flag ada tag baru yang terbaca
            resp[tag][0] = 0xAA;
            resp[tag][1] = 0xAA;
            resp[tag][2] = 0xFF;
            resp[tag][3] = incomingByte;
            respByte = 4; // indeks byte ke-sekian
            for (int i = 0; i < lenData - 1; i++)
            {
              resp[tag][respByte] = Serial2.read(); // baca byte per byte
              respByte++;
            }
            for (int i = 0; i < respByte; i++)
            {
              Serial.printf("%02X", resp[tag][i]);
              Serial.print(" ");
              /*
              %02X : format specifier heksadesimal 2 digit.
              %X : mencetak nilai dalam format heksadesimal tanpa padding. 
              */
            }
            Serial.println();
          }
        }
          // untuk tag yang baru terbaca,
        if (tag > 0 && newTag == 1)
        {
          // pastikan bahwa tag tersebut belum pernah terbaca sebelumnya
          // (memastikan bahwa setiap tag yang terbaca unik)
          for (int i = 0; i < tag; i++)
          {
            if (compareTags(resp[tag], resp[i], 10, 22))
            {
              tag--;
              break;
            }
          }
        }
      }
      flag_COMPLETE_READ = 1;
    }

    else if ((mode == DISPLAY_TAG_MODE) && (!flag_COMPLETE_ONE_CYCLE)) // mode kirim ke serial monitor
    {
      // kirim perintah untuk stop baca tag
      Serial2.write(cmd_stopRead, sizeof(cmd_stopRead));

      // cek CRC sebelum siap ditampilkan
      for (int i = 0; i <= tag; i++)
      {
        byte* row = resp[i];
        int lenRespByte = int(resp[i][3]);
        long CRC = resp[i][lenRespByte + 1] << 8 | resp[i][lenRespByte + 2];

        if (Calculate_CRC(row, lenRespByte + 1) == CRC)
        // Jika CRC sesuai, maka salin tag di variabel 'resp' ke 'readyToSend'
        {
          for (int j = 0; j < lenEPC; j++)
          {
            readyToSend[tagValid][j] = resp[i][j + 10];
          }
          tagValid++;
        }
      }
      Serial.println("RFID Tag terbaca:");

      for (int i = 0; i < tagValid; i++)
      {
        for (int j = 0; j < lenEPC; j++)
        {
          Serial.printf("%02X", readyToSend[i][j]);
          Serial.print(" ");
        }
        Serial.println();
      }
      vTaskDelay(1);
       // re-inisiasi variabel setelah selesai tampil dan akhiri siklus
      reinitVariables();
      flag_COMPLETE_DISPLAY = 1;
      flag_COMPLETE_ONE_CYCLE = 1;
    }
    if (flag_COMPLETE_DISPLAY)
    {
      Serial.println("--- AKHIR SIKLUS ---");
      flag_COMPLETE_DISPLAY = 0;
    }
  }
}

void ledTask(void* pvParameters)
{
  (void)pvParameters;
  while (1) {
  if (mode == READ_TAG_MODE)
  {
    // menyalakan LED hijau sebagai tanda mode baca
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
  }
  else if (mode == DISPLAY_TAG_MODE)
  {
    // menyalakan LED merah sebagai tanda siap tampil pada serial monitor
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
  }
    vTaskDelay(1);
  }
}

//fungsi menghitung CRC
unsigned int Calculate_CRC(unsigned char* ptr, unsigned char len)
{
  unsigned int xorval;
  unsigned char i, j;
  unsigned int CRCacc = 0xffff;
  for (j = 0; j < len; j++)
  {
    for (i = 0; i < 8; i++)
    {
      xorval = ((CRCacc >> 8) ^ (ptr[j] << i)) & 0x0080;
      CRCacc = (CRCacc << 1) & 0xfffe;
      if (xorval)
        CRCacc ^= 0x1021;
    }
  }
  return CRCacc;
}

// fungsi untuk mengecek duplikasi tag
bool compareTags(byte* tag1, byte* tag2, int start, int end)
{
  // memastikan setiap byte pada tag 1 dan 2 sama 
  for (int i = start; i < end; i++)
  {
    if (tag1[i] != tag2[i])
    {
      return false;
    }
  }
  return true;
}

void reinitVariables()
{
  // Re-inisiasi variabel resp
  for (int i = 0; i < maxTag; i++)
  {
    for (int j = 0; j < maxRespByte; j++)
    {
      resp[i][j] = 0;
    }
  }

  // Re-inisiasi variabel readyToSend
  for (int i = 0; i < maxTag; i++)
  {
    for (int j = 0; j < lenEPC; j++)
    {
      readyToSend[i][j] = 0;
    }
  }

  // Re-inisiasi variabel tagValid, mode, dan tag
  tagValid = 0;
  mode = 0;
  tag = -1;
}


void loop()
{
}
