#include <Wire.h>
#include <stdlib.h>
#include <string.h>

#define NVM_CONFIG 0x02
#define EEPROM_CONFIG 0x03
#define VDD 2

int count = 0;
uint8_t slave_address = 0x00;
bool device_present[16] = {false};

uint8_t data_array[16][16] = {};

// Store nvmData in PROGMEM to save on RAM
const char nvmString0[] PROGMEM = "010E000000008E3F0300000000000000";
const char nvmString1[] PROGMEM = "000000000000000000704B1200000000";
const char nvmString2[] PROGMEM = "0000000000000000000000000000E003";
const char nvmString3[] PROGMEM = "00000000000000000028000000000000";
const char nvmString4[] PROGMEM = "00000000000000000000000000000000";
const char nvmString5[] PROGMEM = "00000000000000000000000000000000";
const char nvmString6[] PROGMEM = "00303000303030300000E83030003030";
const char nvmString7[] PROGMEM = "30303033000000000000880000000000";
const char nvmString8[] PROGMEM = "00000200001422300C3C000000000000";
const char nvmString9[] PROGMEM = "08000000000000000000100000000000";
const char nvmString10[] PROGMEM = "00000020000100045D26CC0000020001";
const char nvmString11[] PROGMEM = "00000201000002000100000201000002";
const char nvmString12[] PROGMEM = "00010000020001000000010100000000";
const char nvmString13[] PROGMEM = "00000000000000000000000000000000";
//                               ↓↓ 0 1 2 3 4 5 6 7 8 9 a b c d e f
const char nvmString14[] PROGMEM = "00000000000000000000000000000000";
//                               ↑↑ 0 1 2 3 4 5 6 7 8 9 a b c d e f
const char nvmString15[] PROGMEM = "000000000000000000000000000000A5";

// Store eepromData in PROGMEM to save on RAM
const char eepromString0[] PROGMEM = "10000000000000000000000000000000";
const char eepromString1[] PROGMEM = "01000000000000000000000000000000";
const char eepromString2[] PROGMEM = "00100000000000000000000000000000";
const char eepromString3[] PROGMEM = "00010000000000000000000000000000";
const char eepromString4[] PROGMEM = "00001000000000000000000000000000";
const char eepromString5[] PROGMEM = "00000100000000000000000000000000";
const char eepromString6[] PROGMEM = "00000010000000000000000000000000";
const char eepromString7[] PROGMEM = "00000001000000000000000000000000";
const char eepromString8[] PROGMEM = "00000000100000000000000000000000";
const char eepromString9[] PROGMEM = "00000000010000000000000000000000";
const char eepromString10[] PROGMEM = "00000000001000000000000000000000";
const char eepromString11[] PROGMEM = "00000000000100000000000000000000";
const char eepromString12[] PROGMEM = "00000000000010000000000000000000";
const char eepromString13[] PROGMEM = "00000000000001000000000000000000";
const char eepromString14[] PROGMEM = "00000000000000100000000000000000";
const char eepromString15[] PROGMEM = "00000000000000010000000000000000";

const char *const nvmString[16] PROGMEM = {
    nvmString0,
    nvmString1,
    nvmString2,
    nvmString3,
    nvmString4,
    nvmString5,
    nvmString6,
    nvmString7,
    nvmString8,
    nvmString9,
    nvmString10,
    nvmString11,
    nvmString12,
    nvmString13,
    nvmString14,
    nvmString15};

const char *const eepromString[16] PROGMEM = {
    eepromString0,
    eepromString1,
    eepromString2,
    eepromString3,
    eepromString4,
    eepromString5,
    eepromString6,
    eepromString7,
    eepromString8,
    eepromString9,
    eepromString10,
    eepromString11,
    eepromString12,
    eepromString13,
    eepromString14,
    eepromString15};

////////////////////////////////////////////////////////////////////////////////
// setup
////////////////////////////////////////////////////////////////////////////////
void setup()
{

  Wire.begin(); // join i2c bus (address optional for master)
  Wire.setClock(400000);
  Serial.begin(115200);
  pinMode(VDD, OUTPUT); // This will be the GreenPAK's VDD
  digitalWrite(VDD, HIGH);
  delay(100);
}

////////////////////////////////////////////////////////////////////////////////
// loop
////////////////////////////////////////////////////////////////////////////////
void loop()
{
  String myString = "";
  int incomingByte = 0;
  slave_address = 0x00;
  String NVMorEEPROM = "";

  char selection = query("\nMENU: r = read, e = erase, w = write, p = ping\n");

  switch (selection)
  {
  case 'r':
    Serial.println(F("Reading chip!"));
    requestSlaveAddress();
    NVMorEEPROM = requestNVMorEeprom();
    readChip(NVMorEEPROM);
    // Serial.println(F("Done Reading!"));
    break;
  case 'e':
    Serial.println(F("Erasing Chip!"));
    requestSlaveAddress(); // slave adressを入力させる
    NVMorEEPROM = requestNVMorEeprom();

    if (eraseChip(NVMorEEPROM) == 0)
    {
      // Serial.println(F("Done erasing!"));
    }
    else
    {
      Serial.println(F("Erasing did not complete correctly!"));
    }
    delay(100);
    ping();
    break;
  case 'w':
    Serial.println(F("Writing Chip!"));
    requestSlaveAddress();
    NVMorEEPROM = requestNVMorEeprom();

    if (eraseChip(NVMorEEPROM) == 0)
    {
      // Serial.println(F("Done erasing!"));
    }
    else
    {
      Serial.println(F("Erasing did not complete correctly!"));
    }

    ping();

    if (writeChip(NVMorEEPROM) == 0)
    {
      // Serial.println(F("Done writing!"));
    }
    else
    {
      Serial.println(F("Writing did not complete correctly!"));
    }

    ping();

    readChip(NVMorEEPROM);
    // Serial.println(F("Done Reading!"));
    break;
  case 'p':
    Serial.println(F("Pinging!"));
    ping();
    // Serial.println(F("Done Pinging!"));
    break;
  default:
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////
// request slave address
////////////////////////////////////////////////////////////////////////////////
void requestSlaveAddress()
{
  ping();

  while (1)
  {
    char mySlaveAddress = query("Submit slave address, 0-F: "); // greenpakの初期設定だとslave adressは"1"、ただし前にeraseしていたら"0"になっているはず
    String myString = (String)mySlaveAddress;
    slave_address = strtol(&myString[0], NULL, 16);

    // Check for a valid slave address
    if (device_present[slave_address] == false)
    {
      Serial.println(F("You entered an incorrect slave address. Submit slave address, 0-F: "));
      continue;
    }
    else
    {
      PrintHex8(slave_address);
      Serial.println();
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// request NVM or EEPROM
////////////////////////////////////////////////////////////////////////////////
String requestNVMorEeprom()
{
  while (1)
  {
    char selection = query("MENU: n = NVM, e = EEPROM: ");

    switch (selection)
    {
    case 'n':
      Serial.print(F("NVM"));
      Serial.println();
      return "NVM";
    case 'e':
      Serial.print(F("EEPROM"));
      Serial.println();
      return "EEPROM";
    default:
      Serial.println(F("Invalid selection. You did not enter \"n\" or \"e\"."));
      continue;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// query
////////////////////////////////////////////////////////////////////////////////
char query(String queryString)
{
  Serial.println();

  Serial.print(queryString);
  while (1)
  {
    if (Serial.available() > 0)
    {
      String myString = Serial.readString();
      return myString[0];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// print hex 8
////////////////////////////////////////////////////////////////////////////////
void PrintHex8(uint8_t data)
{
  if (data < 0x10)
  {
    Serial.print("0");
  }
  Serial.print(data, HEX);
}

////////////////////////////////////////////////////////////////////////////////
// readChip
////////////////////////////////////////////////////////////////////////////////
int readChip(String NVMorEEPROM)
{
  int control_code = slave_address << 3;

  if (NVMorEEPROM == "NVM")
  {
    control_code |= NVM_CONFIG;
  }
  else if (NVMorEEPROM == "EEPROM")
  {
    control_code |= EEPROM_CONFIG;
  }

  for (int i = 0; i < 16; i++)
  {
    Wire.beginTransmission(control_code);
    Wire.write(i << 4);
    Wire.endTransmission(false);
    delay(10);
    Wire.requestFrom(control_code, 16);

    while (Wire.available())
    {
      PrintHex8(Wire.read());
    }
    Serial.println();
  }
}

////////////////////////////////////////////////////////////////////////////////
// eraseChip
////////////////////////////////////////////////////////////////////////////////
int eraseChip(String NVMorEEPROM)
{
  int control_code = slave_address << 3;//3bit左シフトさせつつA10,A9,A8を0にして"ERSR register"のBlock Adressに設定している
  int addressForAckPolling = control_code;

  for (uint8_t i = 0; i < 16; i++)
  {
    Serial.print(F("Erasing page: 0x"));
    PrintHex8(i);
    Serial.print(F(" "));

    Wire.beginTransmission(control_code);
    Wire.write(0xE3);//"ERSR register"のWord adressに設定

    if (NVMorEEPROM == "NVM")
    {
      Serial.print(F("NVM "));
      Wire.write(0x80 | i);// 対象ページを指定して削除開始
    }
    else if (NVMorEEPROM == "EEPROM")
    {
      Serial.print(F("EEPROM "));
      Wire.write(0x90 | i);
    }

    /* To accommodate for the non-I2C compliant ACK behavior of the Page Erase Byte, we've removed the software check for an I2C ACK
     *  and added the "Wire.endTransmission();" line to generate a stop condition.
     *  - Please reference "Issue 2: Non-I2C Compliant ACK Behavior for the NVM and EEPROM Page Erase Byte"
     *    in the SLG46824/6 (XC revision) errata document for more information. */

    //    if (Wire.endTransmission() == 0) {
    //      Serial.print(F("ack "));
    //    }
    //    else {
    //      Serial.print(F("nack "));
    //      return -1;
    //    }

    Wire.endTransmission();

    if (ackPolling(addressForAckPolling) == -1)
    {
      return -1;
    }
    else
    {
      Serial.print(F("ready "));
      delay(100);
    }
    Serial.println();
  }

  powercycle();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// writeChip
////////////////////////////////////////////////////////////////////////////////
int writeChip(String NVMorEEPROM)
{
  int control_code = 0x00;
  int addressForAckPolling = 0x00;
  bool NVM_selected = false;
  bool EEPROM_selected = false;

  if (NVMorEEPROM == "NVM")
  {
    // Serial.println(F("Writing NVM"));
    // こののプログラムではNVMにwriteする際に1度eraseされるので、スレーブ アドレスは強制敵に 0x00 になるため0x00で決め打ちする。
    slave_address = 0x00;
    // Set the control code to 0x00 since the chip has just been erased
    control_code = 0x00;
    control_code |= NVM_CONFIG; // adress上位3bitがそれぞれNVM、eeprom、registerに紐づいているため( slave adressはcontorol_code[4bit] + adress上位3bit の8bit)
    NVM_selected = true;
    addressForAckPolling = 0x00; // 不明
  }
  else if (NVMorEEPROM == "EEPROM")
  {
    // Serial.println(F("Writing EEPROM"));
    control_code = slave_address << 3; // slave adressがcontorol_code[4bit] + block adress[3bit]
    control_code |= EEPROM_CONFIG;     // blockaddress 3bitがそれぞれNVM(0x02)、eeprom(0x03)、NVM領域の設定用register(0x01)に紐づいている
    EEPROM_selected = true;
    addressForAckPolling = slave_address << 3;
  }
  // control codeを出力する
  Serial.print(F("Control Code: 0x"));
  PrintHex8(control_code);
  Serial.println();

  // Assign each byte to data_array[][] array;
  // http://www.gammon.com.au/progmem

  // Serial.println(F("New NVM data:"));
  for (size_t i = 0; i < 16; i++)
  {
    // 現在のページの NVM を PROGMEM から取得し、バッファに配置します

    char buffer[64];
    if (NVM_selected)
    {
      // 書き込みたいデータがarudinoのflash領域に書かれているためpgm_read_word()を使ってflash領域にアクセスする
      // ポインタ変数ptrに書き込みたいデータnvmString構造体内のnvmString＊(＊は0~15の任意定数でiで指定される)のアドレスを格納する
      char *ptr = (char *)pgm_read_word(&nvmString[i]);
      strcpy_P(buffer, ptr); // ↑の行で取得したnvmString＊のアドレスの示す内容をbufferにコピー
    }
    else if (EEPROM_selected)
    // やっていることはEEPROMとNVMで同じなため省略
    {
      char *ptr = (char *)pgm_read_word(&eepromString[i]);
      strcpy_P(buffer, ptr);
    }

    for (size_t j = 0; j < 16; j++)
    {
      // flashに書かれているデータはバイナリではなく文字列扱いなので変換が必要⇒一度String class形式に変換して2文字ずつ(1byteずつ)strtol関数で変換してdata_arrayに格納している
      // 実際にgreenpakに書き込まれるのはchar data_array[16][16]の中身
      String temp = (String)buffer[2 * j] + (String)buffer[(2 * j) + 1];
      long myNum = strtol(&temp[0], NULL, 16);
      data_array[i][j] = (uint8_t)myNum;
    }
  }
  Serial.println();

  if (NVM_selected)
  {
    // slave addressを変更する場合に指定できる(greenpakはレジスタの内容でslave addressを変更できる)
    while (1)
    {
      char newSA = query("Enter new slave address 0-F: ");
      String newSAstring = (String)newSA;
      int temp = strtol(&newSAstring[0], NULL, 16);

      if (temp < 0 || temp > 15)
      {
        Serial.println(temp);
        Serial.println(F(" is not a valid slave address."));
        continue;
      }
      else
      {
        slave_address = temp;
        Serial.print(F("0x"));
        PrintHex8(slave_address);
        Serial.println();
        break;
      }
    }
    // 0xCAレジスタにアドレスを書き込むことでSlave addressを変更する(16×16の配列で16を掛け算しているため、greenpakの内部アドレスである2桁の16進数と同じ場所を示せる)
    data_array[0xC][0xA] = slave_address;
  }

  // Write each byte of data_array[][] array to the chip
  for (int i = 0; i < 16; i++)
  {
    Wire.beginTransmission(control_code);
    Wire.write(i << 4);

    PrintHex8(i);
    Serial.print(F(" "));

    for (int j = 0; j < 16; j++)
    {
      Wire.write(data_array[i][j]);
      PrintHex8(data_array[i][j]);
    }

    if (Wire.endTransmission() == 0)
    {
      Serial.print(F(" ack "));
    }
    else
    {
      Serial.print(F(" nack\n"));
      Serial.println(F("Oh No! Something went wrong while programming!"));
      return -1;
    }

    if (ackPolling(addressForAckPolling) == -1)
    {
      return -1;
    }
    else
    {
      Serial.println(F("ready"));
      delay(100);
    }
  }

  powercycle();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// ping
////////////////////////////////////////////////////////////////////////////////
void ping()
{
  delay(100);
  for (int i = 0; i < 16; i++)
  {
    Wire.beginTransmission(i << 3);

    if (Wire.endTransmission() == 0)
    {
      Serial.print(F("device 0x"));
      PrintHex8(i);
      Serial.println(F(" is present"));
      device_present[i] = true;
    }
    else
    {
      device_present[i] = false;
    }
  }
  delay(100);
}

////////////////////////////////////////////////////////////////////////////////
// ack polling
////////////////////////////////////////////////////////////////////////////////
int ackPolling(int addressForAckPolling)
{
  int nack_count = 0;
  while (1)
  {
    Wire.beginTransmission(addressForAckPolling);
    if (Wire.endTransmission() == 0)
    {
      return 0;
    }
    if (nack_count >= 1000)
    {
      Serial.println(F("Geez! Something went wrong while programming!"));
      return -1;
    }
    nack_count++;
    delay(1);
  }
}

////////////////////////////////////////////////////////////////////////////////
// power cycle
////////////////////////////////////////////////////////////////////////////////
void powercycle()
{
  Serial.println(F("Power Cycling!"));
  digitalWrite(VDD, LOW);
  delay(500);
  digitalWrite(VDD, HIGH);
  // Serial.println(F("Done Power Cycling!"));
}
