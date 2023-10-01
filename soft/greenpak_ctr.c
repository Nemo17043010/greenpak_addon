
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
#define MAX_LINE_LENGTH 256

uint8_t slave_address = 0x01; //greenpakの初期値は0x01

uint8_t data_array[16][16] = {};
static const char *dev_name = "/dev/i2c-3";
char *comp_str = {"NVM", "EEPROM"};
size_t i, j;

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
    uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t* length)
{
  int ret;
  /* I2Cデバイスをオープンする. */
  int32_t fd = open(dev_name, O_RDWR);
  
  if (fd == -1)
  {
    fprintf(stderr, "i2c_write: failed to open: %s \n", strerror(errno));
    return -1;
  }

  /* I2C-Write用のバッファを準備する. (lengthに+1しているのはレジスタアドレスも送信する必要があるため)*/
  uint8_t *buffer = (uint8_t *)malloc(length + 1);
  if (buffer == NULL)
  {
    fprintf(stderr, "i2c_write: failed to memory allocate\n");
    close(fd);
    return -1;
  }
  buffer[0] = reg_addr;             /* 1バイト目にレジスタアドレスをセット. */
  memcpy(&buffer[1], data, length); /* 2バイト目以降にデータをセット. */

  /* I2C-Writeメッセージを作成する.(lengthに+1しているのはレジスタアドレスも送信する必要があるため) */
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

/*! NVM領域かEEPROM領域いずれかの内容をdumpする
 * @param[in] NVMorEEPROM 対象を示す変数
 */
int readchip(char *NVMorEEPROM)
{
  uint8_t control_code = slave_address << 3;

  if (strcmp(NVMorEEPROM, "NVM"))
  {
    control_code |= NVM_CONFIG;
  }
  else if (strcmp(NVMorEEPROM, "EEPROM"))
  {
    control_code |= EEPROM_CONFIG;
  }else{
    perror("kokoni kuru hazu nai\n");
    return EXIT_FAILURE;
  }

  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 16; j++)
    {
      i2c_read(control_code, ((i << 4) + j), data_array[i] + j, 1);
       //i:上位4bit j:下位4bit data_array[i]+j:データを格納する配列のポインタ 
      fprintf(stderr, "%x :", ((i << 4) + j));
      snprintf(NULL, 0, "%x \n", data_array[i][j]);
    }
  }
  return 0;
}

/*! NVM領域かEEPROM領域いずれかの内容を消去する
 * @param[in] NVMorEEPROM 対象を示す変数
 */
void erasechip(char *NVMorEEPROM)
{
  uint8_t control_code = slave_address << 3;
  uint8_t addressForAckPolling = control_code;
  size_t length = 1;

  for (i = 0; i < 16; i++)
  {
    fprintf(stderr, "erasing page:%x", i);

    if (strcmp(NVMorEEPROM,"NVM") )
    {
      fprintf(stderr, "NVM");
      control_code = 0x00;
      control_code |= NVM_CONFIG; // adress上位3bitがそれぞれNVM、eeprom、registerに紐づいているため( slave adressはcontorol_code[4bit] + adress上位3bit の8bit)
      i2c_write(control_code, 0xE3, 0x80 | i, &length);
    }
    else if (strcmp(NVMorEEPROM,"EEPROM"))
    {
      fprintf(stderr, "EEPROM");
      i2c_write(control_code, 0xE3, 0x90 | i, &length);
    }
    usleep(40000); //消去時間がmax 20msらしいが念のため20ms待つことにする
  }
  
  slave_address = 0x00 // 上の消去処理でslaveアドレスを決めているレジスタ"0xCA"も消してしまっているから
  
}


/*! NVM領域かEEPROM領域いずれかにcsvファイルに基づいた内容を書き込む
 * @param[in] NVMorEEPROM 対象を示す変数
 * @param[in] csv_path 書き込むcsvファイルのパス
 */
int writechip(char* NVMorEEPROM, char* csv_path)
{
  uint8_t control_code;
  size_t array_size;
  uint8_t* data = read_csv(csv_path, &array_size);

  
  if (strcmp(NVMorEEPROM, "NVM"))
  {
    erasechip(NVMorEEPROM);
    control_code = slave_address << 3;
    control_code |= NVM_CONFIG;
  }
  else if (strcmp(NVMorEEPROM, "EEPROM"))
  {
    erasechip(NVMorEEPROM);
    control_code = slave_address << 3;
    control_code |= EEPROM_CONFIG;
  }else{
    perror("kokoni kuru hazu nai\n");
    return EXIT_FAILURE;
  }

  array_size = 16;

  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 16; j++)
    {
      i2c_write(control_code, ((i << 4) + j), data_array[i] + j, &array_size);
       //i:上位4bit j:下位4bit data_array[i]+j:データを格納する配列のポインタ 
    }
  }
  soft_reset();
  return 0;


}



/*! CSVファイルからuint8_tの配列を読み取る関数
 * @param[in] filename 読み込むcsvファイル名
 * @param[in,out] array_size データの入った配列のサイズが入る 
 */
// CSVファイルからuint8_tの配列を読み取る関数
uint8_t* read_csv(const char* filename, size_t* array_size) {
    FILE* file = fopen(filename, "r");
    
    if (!file) {
        perror("An error occurred while opening the file.");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    uint8_t* data_array = NULL;
    size_t current_index = 0;
    char *end = NULL;
    
    while (fgets(line, sizeof(line), file) != NULL) {
        // 改行文字を削除
        size_t line_length = strlen(line);
        if (line_length > 0 && line[line_length - 1] == '\n') {
            line[line_length - 1] = '\0';
        }
        
        // 16進数文字列からuint8_tに変換して配列に格納
        uint8_t num = (uint8_t)strtol(line, end, 16);
        if (*end != NULL)
        {
            printf("Error! Contains invalid characters!");
            return -1;
        }
        data_array = realloc(data_array, (current_index + 1) * sizeof(uint8_t));
        if (!data_array) {
            perror("Memory allocation error.");
            fclose(file);
            return NULL;
        }
        data_array[current_index] = num;
        current_index++;
    }

    fclose(file);
    *array_size = current_index;
    return data_array;
}

uint8_t soft_reset(){
  //レジスタ C8 [1601] (I2C リセット ビット)を「1」に設定する
  uint8_t tmp;
  size_t length = 1;
  uint8_t control_code = slave_address << 3;
  control_code |= NVM_CONFIG;
  if(i2c_read(control_code, 0xC8, &tmp,1)){
    printf("i2c communication failed. \n")
    return 1;
  }
  if(i2c_write(control_code, 0xC8, tmp | 0x01, &length)){
    printf("Failed to write to [1601]bit \n")
    return 1;
  }
  usleep(500000);

}





/*! main関数の引数
 * @param[in] argv[1] 書き込むcsvファイルのパス
 * @param[in] argv[2] NVM or EEPROM 
 * @param[in] argv[3] r or w or e 読み込み 書き込み 消去
 */
int main(int argc, char *argv[]) {
    uint8_t tmp;
    if (argc < 4) {
        printf("Insufficient arguments.\n");
        return EXIT_FAILURE;
    }

    // Branch based on the first argument
    if (strcmp(argv[2], "NVM") == 0) {
        printf("Option 1, NVM selected. /n");
        // Processing for the case when the first argument is "option1"
        if (strcmp(argv[3], "-r") == 0) {
            printf("Option 1, Sub-option -r selected.\n");
            readchip('NVM');

        } else if (strcmp(argv[3], "-w") == 0) {
            printf("Option 1, Sub-option -w selected.\n");
            writechip('NVM', argv[1]);

        } else if (strcmp(argv[3], "e") == 0) {
            printf("Option 1, Sub-option -e selected.\n");
            erasechip('NVM');

        } else {
            printf("Invalid third argument.\n");
            return EXIT_FAILURE;
        }
    } else if (strcmp(argv[2], "EEPROM") == 0) {
        printf("Option 1, EEPROM selected. /n");
        // Processing for the case when the first argument is "option2"
        if (strcmp(argv[3], "-r") == 0) {
            printf("Option 2, Sub-option X selected.\n");
        } else if (strcmp(argv[3], "-w") == 0) {
            printf("Option 2, Sub-option Y selected.\n");
        } else if (strcmp(argv[3], "-e") == 0) {
            printf("Option 2, Sub-option Z selected.\n");
        } else {
            printf("Invalid third argument.\n");
            return EXIT_FAILURE;
        }
    } else if (strcmp(argv[1], "reset") == 0){
      printf("green pak resetting...");
      if(soft_reset()){
        printf("Failed to reset. \n");
      }
      printf("success!!\n");

    } else {
        printf("Invalid secound argument.\n");
        printf(" [csv file name] ['NVM' or 'EEPROM' ] ['-r' or '-w' or '-e']\n");
        return;
    }

    return 0;
}
