
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#define NVM_CONFIG 0x02
#define EEPROM_CONFIG 0x03
#define VDD 2

uint8_t slave_address = 0x01;

uint8_t data_array[16][16] = {};
static const char *dev_name = "/dev/i2c-3";
uint8_t i, j;

/*! I2Cスレーブデバイスからデータを読み込む.
 * @param[in] dev_addr デバイスアドレス.
 * @param[in] reg_addr レジスタアドレス.
 * @param[out] data 読み込むデータの格納場所を指すポインタ.
 * @param[in] length 読み込むデータの長さ.
 */
int8_t i2c_read(
    uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t length)
{
  /* I2Cデバイスをオープンする. */
  int32_t fd = open(dev_name, O_RDWR);
  if (fd == -1)
  {
    fprintf(stderr, "i2c_read: failed to open: %s\n", strerror(errno));
    return -1;
  }

  /* I2C-Readメッセージを作成する. */
  struct i2c_msg messages[] = {
      {dev_addr, 0, 1, &reg_addr},        /* レジスタアドレスをセット. */
      {dev_addr, I2C_M_RD, length, data}, /* dataにlengthバイト読み込む. */
  };
  struct i2c_rdwr_ioctl_data ioctl_data = {messages, 2};

  /* I2C-Readを行う. */
  if (ioctl(fd, I2C_RDWR, &ioctl_data) != 2)
  {
    fprintf(stderr, "i2c_read: failed to ioctl: %s\n", strerror(errno));
    close(fd);
    return -1;
  }

  close(fd);
  return 0;
}

/*! I2Cスレーブデバイスにデータを書き込む.
 * @param[in] dev_addr デバイスアドレス.
 * @param[in] reg_addr レジスタアドレス.
 * @param[in] data 書き込むデータの格納場所を指すポインタ.
 * @param[in] length 書き込むデータの長さ.
 */
int8_t i2c_write(
    uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t length)
{
  int ret;
  /* I2Cデバイスをオープンする. */
  int32_t fd = open(dev_name, O_RDWR);
  if (fd == -1)
  {
    fprintf(stderr, "i2c_write: failed to open: %s\n", strerror(errno));
    return -1;
  }

  /* I2C-Write用のバッファを準備する. */
  uint8_t *buffer = (uint8_t *)malloc(length + 1);
  if (buffer == NULL)
  {
    fprintf(stderr, "i2c_write: failed to memory allocate\n");
    close(fd);
    return -1;
  }
  buffer[0] = reg_addr;             /* 1バイト目にレジスタアドレスをセット. */
  memcpy(&buffer[1], data, length); /* 2バイト目以降にデータをセット. */

  /* I2C-Writeメッセージを作成する. */
  struct i2c_msg message = {dev_addr, 0, length + 1, buffer};
  struct i2c_rdwr_ioctl_data ioctl_data = {&message, 1};

  /* I2C-Writeを行う. */
  if (ioctl(fd, I2C_RDWR, &ioctl_data) != 1)
  {
    fprintf(stderr, "i2c_write: failed to ioctl: %s\n", strerror(errno));
    free(buffer);
    close(fd);
    return -1;
  }

  free(buffer);
  close(fd);
  return 0;
}

int readChip(char *NVMorEEPROM)
{
  uint8_t control_code = slave_address << 3;

  if (strcmp(NVMorEEPROM, "NVM"))
  {
    control_code |= NVM_CONFIG;
  }
  else if (strcmp(NVMorEEPROM, "EEPROM"))
  {
    control_code |= EEPROM_CONFIG;
  }

  for (int i = 0; i < 16; i++)
  {
    for (int j = 0; j < 16; j++)
    {
      i2c_read(control_code, ((i << 4) + j), data_array[i] + j, 1);
      fprintf(stderr, "%x :", ((i << 4) + j));
      snprintf(NULL, 0, "%x", data_array[i][j]);
    }
  }
  return 0;
}

int eraseChip(char *NVMorEEPROM)
{
  uint8_t control_code = slave_address << 3;
  uint8_t addressForAckPolling = control_code;

  for (i = 0; i < 16; i++)
  {
    fprintf(stderr, "erasing page:%x", i);

    if (NVMorEEPROM == "NVM")
    {
      fprintf(stderr, "NVM");
      i2c_write(control_code, 0xE3, 0x80 | i, 1);
    }
    else if (NVMorEEPROM == "EEPROM")
    {
      fprintf(stderr, "EEPROM");
      i2c_write(control_code, 0xE3, 0x90 | i, 1);
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

int writeChip(char *NVMorEEPROM)
{
}

void main(int argc, char *argv[])
{
  FILE *fp;
  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    return EXIT_FAILURE;
  }

  fp = fopen(argv[1], "r");
  if (fp == NULL)
  {
    fprintf(stderr, "Error: cannot open file '%s'\n", argv[1]);
    return EXIT_FAILURE;
  }
}
