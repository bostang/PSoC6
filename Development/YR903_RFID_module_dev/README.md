# YR903 RFID Module Dev

Modul RFID UHF.

## Informasi terkait protokol komunikasi
- Baud rade (default): 115200 bps (8N1)
- 


## Command

1. 
```C++
const byte cmd_reset[] = {0xA0, 0x03, 0x01, 0x70, 0xEC};

// real-time inventory (read)
const byte cmd_Read[] = {0xA0, 0x04, 0x01, 0x89, 0x01, 0xD1};

// buffer mode tag inventory (read and store to buffer)
const byte cmd_storeToBuffer[] = {0xA0,0x04, 0x01, 0x80, 0x01, 0xDA};

// read / get data from buffer
const byte readBuffer[]= {0xA0, 0x03, 0x01, 0x90, 0xCC};

// read / get data from buffer and then clear it
const byte readnClearBuffer[]= {0xA0, 0x03, 0x01, 0x91, 0xCB};

// menulis ulang isi tag

```


## Catatan

1. **ISO18000-6B** : operation in the frequency range of  **860-960MHZ**.
2. **ISO 18000-6C**: communication standards set for UHF Class 1 Gen 2 ITF or Interrogator-Talks-First RFID readers and tags. 
3. **RSSI** (Received signal strength indicator) -> satuan pengukuran signal strength (alternatif dBm)
4. **DRM** (dense reader mode) :  category of RF Modes in which readers are configured to minimize out-of-band interference with other readers and tags, enabling many readers to operate in the same physical location.
Coverage area - The amount of
5. **CRC (code rule  check)** :Cyclic Redundancy Check --> whether the received EPC is correct or not, as a verification of the wireless link between the tag and the reader.
6. **EPC** (electronic product code)
7. **PC (protocol check)** : 
8. word (16 bits), byte (8 bits)
9. **UID** : unique identifier



sebuah langkah untuk bisa mengirimkan sekuens data secara pas dari PSoC menuju modul YR903 UHF RFID module :
1. coba untuk mengirimkan arrya of byte yang sama kepada serial monitor supaya kita bisa tahu apakah data yang kita kirimkan sudah pas atau belum.
   a. mengirimkannya langsung ke terminal : pin P5_0 (receiver) dari P5_1 (transmitter)
   b. mengirimkannya dari pin P9_1 (yang nantinya akan dihubungkan ke RFID module) ke P5_0 (supaya bisa dilihat dari terminal)
   c. ulangi langkah a dan b namun sekarang muenggunakan usb-to-ttl.    

   if the data cannot be displayed in hexadecimal format, you can use termite and connecrt usb TTL .