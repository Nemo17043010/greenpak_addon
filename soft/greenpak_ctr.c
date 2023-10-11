
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
#include <errno.h>
#include <time.h>


#define REG_CONFIG 0x00
#define NVM_CONFIG 0x02
#define EEPROM_CONFIG 0x03


int8_t i2c_read(
    uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t length);
int8_t i2c_write(
    uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t length);
int8_t i2c_write_for_erase(
    uint8_t dev_addr, uint8_t reg_addr, const uint8_t* data, uint16_t length);
int8_t readchip(char *NVMorEEPROM);
int8_t erasechip(char *NVMorEEPROM);
int8_t writechip(char* NVMorEEPROM, char* csv_path);
uint8_t* read_csv(const char* filename, uint16_t* array_size);
uint8_t** convert_to_2d_array(const uint8_t* input);
uint8_t soft_reset();




uint8_t slave_address = 0x01; //greenpakの初期値は0x01

static const char *dev_name = "/dev/i2c-3";
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
int8_t i2c_write_for_erase(
    uint8_t dev_addr, uint8_t reg_addr, const uint8_t* data, uint16_t length)
{
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
  ioctl(fd, I2C_RDWR, &ioctl_data);

  free(buffer);
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
    uint8_t dev_addr, uint8_t reg_addr, const uint8_t* data, uint16_t length)
{
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
int8_t readchip(char *NVMorEEPROM)
{
  uint8_t control_code = slave_address << 3;
  uint8_t data_array[16][16] = {};

  if (strcmp(NVMorEEPROM, "NVM") == 0)
  {
    control_code |= NVM_CONFIG;
  }
  else if (strcmp(NVMorEEPROM, "EEPROM") == 0)
  {
    control_code |= EEPROM_CONFIG;
  }else{
    perror("kokoni kuru hazu nai\n");
    return -1;
  }

  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 16; j++)
    {

      if (i2c_read(control_code, ((i << 4) + j), data_array[i] + j, 1)!=0)
      {
        printf("i2c_read error \n");
        return -1;
      }

       //i:上位4bit j:下位4bit data_array[i]+j:データを格納する配列のポインタ 
      fprintf(stderr, "%x :", ((i << 4) + j));
      fprintf(stderr, "%x \n", data_array[i][j]);
    }
  }
  return 0;
}


/*! NVM領域かEEPROM領域いずれかにcsvファイルに基づいた内容を書き込む
 * @param[in] NVMorEEPROM 対象を示す変数
 * @param[in] csv_path 書き込むcsvファイルのパス
 */
int8_t writechip(char* NVMorEEPROM, char* csv_path)
{
  uint8_t control_code;
  uint16_t array_size;
  printf("csv file read\n");
  uint8_t* data = read_csv(csv_path, &array_size);
  printf("convert 2d array \n");
  uint8_t** data_array = convert_to_2d_array(data);
  if (!data_array) {
    printf("Incorrect CSV file.\n");
        return 1;
  }
  
  if (strcmp(NVMorEEPROM, "NVM") == 0)
  {

    if (erasechip(NVMorEEPROM)!=0)
    {
      return -1;
    }

    control_code = slave_address << 3;
    control_code |= NVM_CONFIG;
  }
  else if (strcmp(NVMorEEPROM, "EEPROM") == 0)
  {
    if (erasechip(NVMorEEPROM)!=0)
    {
      return -1;
    }

    control_code = slave_address << 3;
    control_code |= EEPROM_CONFIG;
  }else{
    perror("kokoni kuru hazu nai\n");
    return EXIT_FAILURE;
  }

  array_size = 16;

  for (i = 0; i < 16; i++)
  {
    printf("Write the following 16 bytes\n");
    printf("-------------\n");
    for(j = 0 ; j < 16; j++){
      printf("adress:%x, value:%x \n",((i << 4) + j), data_array[i][j]);
    }
    printf("-------------\n");
    printf("i2c_write \n");

    if (i2c_write(control_code, (i << 4), data_array[i], array_size)!=0)
    {
      printf("i2c_write error \n");
      return -1;
    }

    //i:上位4bit j:下位4bit data_array[i]+j:データを格納する配列のポインタ
    usleep(60000);
    if (i != 15 && j != 15)
    {
      printf("next page \n");
      printf("\n");
    }else{
      printf("end write\n");
    }

  }

  ;
  if (soft_reset()!=0)
  {
    printf("reset error \n");
    return -1;
  }

  return 0;


}


/*! NVM領域かEEPROM領域いずれかの内容を消去する
 * @param[in] NVMorEEPROM 対象を示す変数
 */
int8_t erasechip(char *NVMorEEPROM)
{
  uint8_t control_code = slave_address << 3; //3bit左シフトさせつつA10,A9,A8を0にして"ERSR register"のBlock Adressに設定している
  uint8_t addressForAckPolling = control_code;
  uint16_t length = 1;
  uint8_t tmp;

  for (i = 0; i < 16; i++)
  {
    
    fprintf(stderr, "erasing page:%x \n", i);

    if (strcmp(NVMorEEPROM,"NVM") == 0)
    {
      tmp = 0x80 | i;
      if (tmp == 0xca)
      {
        continue;
      }
   

      if (i2c_write_for_erase(control_code, 0xE3, &tmp, length)!=0)
      {
        printf("NVM erase error \n");
        return -1;
      }

    }
    else if (strcmp(NVMorEEPROM,"EEPROM") == 0)
    {
      fprintf(stderr, "EEPROM");
      tmp = 0x90 | i;

      if (i2c_write_for_erase(control_code, 0xE3, &tmp , length)!=0)
      {
        printf("EEPROM erase error \n");
        return -1;
      }

    }
    usleep(40000); //消去時間がmax 20msらしいが念のため20ms待つことにする
    return 0;
  }

  //slave_address = 0x00; // 上の消去処理でslaveアドレスを決めているレジスタ"0xCA"も消してしまっているから-> contnueで飛ばしてみる

}

uint8_t soft_reset(){
  //レジスタ C8 [1601] (I2C リセット ビット)を「1」に設定する
  uint8_t tmp;
  uint16_t length = 1;
  uint8_t control_code = slave_address << 3;
  control_code |= NVM_CONFIG;
  if(i2c_read(control_code, 0xC8, &tmp,1)){
    printf("i2c communication failed. \n");
    return 1;
  }
  tmp = tmp | 0x02;
  if(i2c_write(control_code, 0xC8, &tmp , length)){
    printf("Failed to write to [1601]bit \n");
    return 1;
  }
  usleep(500000);
  return 0;

}

#define MAX_LINE_LENGTH 256
/*! CSVファイルからuint8_tの配列を読み取る関数
 * @param[in] filename 読み込むcsvファイル名
 * @param[in,out] array_size データの入った配列のサイズが入る 
 */
// CSVファイルからuint8_tの配列を読み取る関数
uint8_t* read_csv(const char* filename, uint16_t* array_size) {
    FILE* file = fopen(filename, "r");
    
    if (!file) {
        perror("An error occurred while opening the file.");
        return NULL;
    }

    char line[MAX_LINE_LENGTH];
    uint8_t* data_array = NULL;
    size_t current_index = 0;
    char *end;
    
    while (fgets(line, sizeof(line), file) != NULL) {
        // 改行文字を削除
        size_t line_length = strlen(line);
        if (line_length > 0 && line[line_length - 1] == '\n') {
            line[line_length - 1] = '\0';
        }
      
        // 16進数文字列からuint8_tに変換して配列に格納
        uint8_t num = (uint8_t)strtol(line, &end, 16);   //
        if (*end != '\0')
        {
            printf("Error! Contains invalid characters!\n");
            return NULL;
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

#define INPUT_SIZE 256
#define OUTPUT_ROWS 16
#define OUTPUT_COLS 16

// 1次元配列を2次元配列に変換する関数
uint8_t** convert_to_2d_array(const uint8_t* input)
{
    uint8_t** output = (uint8_t**)malloc(OUTPUT_ROWS * sizeof(uint8_t*));
    if (!output) {
        perror("Memory allocation error");
        return NULL;
    }

    for (i = 0; i < OUTPUT_ROWS; i++) {
        output[i] = (uint8_t*)malloc(OUTPUT_COLS * sizeof(uint8_t));
        if (!output[i]) {
            perror("Memory allocation error");
            for (j = 0; j < i; j++) {
                free(output[j]);
            }
            free(output);
            return NULL;
        }
    }

    size_t input_index = 0;
    for (i = 0; i < OUTPUT_ROWS; i++) {
        for (j = 0; j < OUTPUT_COLS; j++) {
            output[i][j] = input[input_index];
            input_index++;
        }
    }

    return output;
}








/*! main関数の引数
* @param[in] argv[1] NVM or EEPROM 
* @param[in] argv[2] r or w or e 読み込み 書き込み 消去
* @param[in] argv[3] 書き込むcsvファイルのパス
*/
int8_t main(int argc, char *argv[]) {
    uint8_t tmp;
    if (argc < 4) {
        printf("Insufficient arguments.\n");
        printf(" ['NVM' or 'EEPROM] ['-r' or '-w' or '-e'] [csv file name]\n");
        return EXIT_FAILURE;
    }

    // Branch based on the first argument
    if (strcmp(argv[1], "NVM") == 0) {
        printf("Option 1, NVM selected. \n");
        // Processing for the case when the first argument is "option1"
        if (strcmp(argv[2], "-r") == 0) {
            printf(" Sub-option -r selected.\n");
            if (readchip(argv[1]!=0))
            {
              printf("readchip error!\n");
              return -1;
            }


        } else if (strcmp(argv[2], "-w") == 0) {
            printf("Option 1, Sub-option -w selected.\n");

            if (writechip(argv[1], argv[3])!=0)
            {
              printf("writechip error!\n");
              return -1;
            }


        } else if (strcmp(argv[2], "-e") == 0) {
            printf("Option 1, Sub-option -e selected.\n");

            if (erasechip(argv[1])!=0)
            {
              printf("erasechip error!\n");
              return -1;
            }


        } else {
            printf("Invalid third argument.\n");
            printf(" ['NVM' or 'EEPROM] ['-r' or '-w' or '-e'] [csv file name]\n");
            return EXIT_FAILURE;
        }
    } else if (strcmp(argv[1], "EEPROM") == 0) {
        printf("Option 1, EEPROM selected. /n");
        // Processing for the case when the first argument is "option2"
        if (strcmp(argv[2], "-r") == 0) {
            printf(" Sub-option -r selected.\n");

            if (readchip(argv[1])!=0)
            {
              printf("readchip error!\n");
              return -1;
            }

        } else if (strcmp(argv[2], "-w") == 0) {
            printf("Option 1, Sub-option -w selected.\n");

            if (writechip(argv[1], argv[3])!=0)
            {
              printf("writechip error!\n");
              return -1;
            }


        } else if (strcmp(argv[2], "-e") == 0) {
            printf("Option 1, Sub-option -e selected.\n");

            if (erasechip(argv[1])!=0)
            {
              printf("erasechip error!\n");
              return -1;
            }


        } else {
            printf("Invalid third argument.\n");
            printf(" ['NVM' or 'EEPROM] ['-r' or '-w' or '-e'] [csv file name]\n");
            return -1;
        }
    } else {
        printf("Invalid secound argument.\n");
        printf(" ['NVM' or 'EEPROM] ['-r' or '-w' or '-e'] [csv file name]\n");
        return -1;
    }

    return 0;
}
