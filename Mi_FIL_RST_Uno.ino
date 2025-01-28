// ============================================================
// Include
// ============================================================
#include <stdio.h>

#include <SPI.h>
#include <MFRC522.h>

#include "utils_sha1.h"

// ============================================================
// Define
// ============================================================
#define SS_PIN  10
#define RST_PIN 9

#define UID_CHAR_LEN      14
#define UID_BYTE_LEN      (UID_CHAR_LEN / 2)
#define PSW_LEN           4
#define RFID_ACK_LEN      2
#define SHA1_RESULT_LEN   20

#define BLK_ADDR_READ     8
#define BLK_RBUF_SIZE     18

#define BLK_ADDR_WRITE    8
#define BLK_WBUF_SIZE     16

// ============================================================
// Variable
// ============================================================
MFRC522 mfrc522;  
MFRC522::PICC_Type  mfrc522_piccType;
MFRC522::StatusCode mfrc522_status;

byte a08_nfc_psw[]  = {0x00, 0x00, 0x00, 0x00};
byte a08_nfc_ack[]  = {0x00, 0x00};             // NFCtag 返回的 16 位密码 ACK。

byte u08_block_read_addr   = BLK_ADDR_READ;
byte u08_block_read_size   = BLK_RBUF_SIZE;
byte a08_block_read_buf[BLK_RBUF_SIZE];

byte u08_block_write_addr   = BLK_ADDR_WRITE;
byte u08_block_write_size   = BLK_WBUF_SIZE;
byte a08_block_write_buf[BLK_WBUF_SIZE];

uint8_t u08_idx = 0;

bool b01_data_zero = false;

// ============================================================
// Function
// ============================================================
void dump_byte_array(byte* p08_buf, byte u08_buf_size) 
{
  byte u08_idx;
  
  for (u08_idx = 0; u08_idx < u08_buf_size; u08_idx ++) 
  {
    Serial.print(p08_buf[u08_idx] < 0x10 ? " 0" : " ");
    Serial.print(p08_buf[u08_idx], HEX);
  }
}

void rfid_password_calculate(byte *p08_uid, byte *p08_psw) 
{
  byte a08_sha1[SHA1_RESULT_LEN];

  utils_sha1(p08_uid, UID_BYTE_LEN, a08_sha1);

  p08_psw[0] = a08_sha1[(a08_sha1[0]     ) % SHA1_RESULT_LEN];
  p08_psw[1] = a08_sha1[(a08_sha1[0] + 5 ) % SHA1_RESULT_LEN];
  p08_psw[2] = a08_sha1[(a08_sha1[0] + 13) % SHA1_RESULT_LEN];
  p08_psw[3] = a08_sha1[(a08_sha1[0] + 17) % SHA1_RESULT_LEN];
}

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  SPI.begin();

  mfrc522.PCD_Init(SS_PIN, RST_PIN); 

  Serial.print("\n");
  Serial.print("Xiaomi Air Purifier (1, 2, 3, Pro) Filter RFID Reset\n");
  Serial.print("Initialize Finish\n");
}

// ============================================================
// Loop Function
// ============================================================
void loop() 
{
  delay(2000);

  // Find Card
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
  {
    Serial.print("No Card\n\n");
    return;
  }
  else
  {
    Serial.print("Card Find, UID：");
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));    
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.print("\n");
  }

  // Check Card Type
  mfrc522_piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.print(mfrc522.PICC_GetTypeName(mfrc522_piccType));
  Serial.print("\n");

  if (mfrc522_piccType != MFRC522::PICC_TYPE_MIFARE_UL) 
  {
    Serial.print("Card Type is not MIFARE_UL\n");
    return;
  }
  else
  {
    Serial.print("Card Type is MIFARE_UL\n");
  }

  // RFID Password Calculate
  rfid_password_calculate(mfrc522.uid.uidByte, a08_nfc_psw);
  Serial.print("RFID Password:");  
  dump_byte_array(a08_nfc_psw, PSW_LEN);
  Serial.print("\n");

  // RFID Auth Test
  mfrc522_status = mfrc522.PCD_NTAG216_AUTH(a08_nfc_psw, a08_nfc_ack);

  if (mfrc522_status != MFRC522::STATUS_OK) 
  {
    Serial.print("AUTH Failed\n");  
    return;
  }
  else
  {
    Serial.print("AUTH Success\n");  
  }

  // RFID Read 
  u08_block_read_addr = BLK_ADDR_READ;
  u08_block_read_size = sizeof(a08_block_read_buf);

  mfrc522_status = mfrc522.MIFARE_Read(u08_block_read_addr, a08_block_read_buf, &u08_block_read_size);

  if (mfrc522_status != MFRC522::STATUS_OK) 
  {
    Serial.print(F("Read Failed\n"));
    return;
  }
  else
  {
    Serial.print("Read block addr 8 Success\n");
    Serial.print("Data:");  
    dump_byte_array(a08_block_read_buf, 16);
    Serial.print("\n");
  }
  
  // Life Check
  b01_data_zero = true;

  for (u08_idx = 0; u08_idx < 16; u08_idx ++)
  {
    if (a08_block_read_buf[u08_idx] != 0x00)
      b01_data_zero = false;
  }

  if (b01_data_zero == true)
  {
    Serial.print("RFID Block 8 is already 0x00.\n");  
    return;
  }

  // RFID Write 
  u08_block_write_addr = BLK_ADDR_WRITE;
  u08_block_write_size = sizeof(a08_block_write_buf);  

  for (u08_idx = 0; u08_idx < u08_block_write_size; u08_idx ++)
  {
    a08_block_write_buf[u08_idx] = 0x00;
  }

  mfrc522_status = mfrc522.MIFARE_Write(u08_block_write_addr, a08_block_write_buf, u08_block_write_size);

  if (mfrc522_status != MFRC522::STATUS_OK) 
  {
     Serial.print("Write Failed\n");
     return;
  }
  else
  {
    Serial.print("Write Sueccess\n");
  }
}


