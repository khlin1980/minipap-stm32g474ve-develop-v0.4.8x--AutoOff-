/**
 * @file    bsp_w25q64.c
 * @brief   W25Q64驅動程式
 * @author  Mculover666(http://www.mculover666.cn)
 * @note    此驅動程式基於STM32CubeMX產生的QSPI初始化工程
 */

#include "bsp_w25q64.h"

#define QSPI_FLASH_PAGESIZE         256
#define QSPI_FLASH_SECTOR           4096

//* must be change 2 define when model changed
#define QSPI_FLASH_ID               0xEF4018        // for w25Q128Mb  
#define QSPI_FLASH_TOTAL_SIZE       (0x1000000 - 1) 

#define QSPI_FLASH_WriteEnable      0x06
#define QSPI_FLASH_WriteDisable     0x04
#define QSPI_FLASH_ReadStatusReg    0x05
#define QSPI_FLASH_WriteStatusReg   0x01
#define QSPI_FLASH_ReadData         0x03
#define QSPI_FLASH_FastReadData     0x0B
#define QSPI_FLASH_FastReadDual     0x3B
#define QSPI_FLASH_PageProgram      0x02
#define QSPI_FLASH_BlockErase       0xD8
#define QSPI_FLASH_SectorErase      0x20
#define QSPI_FLASH_ChipErase        0xC7
#define QSPI_FLASH_PowerDown        0xB9
#define QSPI_FLASH_ReleasePowerDown 0xAB
#define QSPI_FLASH_DeviceID         0xAB
#define QSPI_FLASH_ManufactDeviceID 0x90
#define QSPI_FLASH_JedecDeviceID    0x9F
#define QSPI_FLASH_WIP_FLAG         0x01
#define QSPI_FLASH_DUMMY_BYTE       0xFF

#define CHOOSE_BIT_16 16
#define CHOOSE_BIT_8 8

#define CHECK_RET_RETURN(ret) \
    do                        \
    {                         \
        if ((ret) < 0)        \
        {                     \
            return ret;       \
        }                     \
    } while (0)


// 定義QSPI控制代碼，這裡保留使用cubeMX產生的變數命名，方便使用者參考和移植
extern QSPI_HandleTypeDef hqspi1;
#define QSPI_W25Q  hqspi1  

//* external function from other files
extern void Error_Handler(void);

/**
 * @brief	QSPI發送命令
 *
 * @param   instruction		要發送的指令
 * @param   address			發送到的目的地址
 * @param   dummyCycles		空指令週期數
 * @param   instructionMode	指令模式;QSPI_INSTRUCTION_NONE,QSPI_INSTRUCTION_1_LINE,QSPI_INSTRUCTION_2_LINE,QSPI_INSTRUCTION_4_LINE
 * @param   addressMode		地址模式; QSPI_ADDRESS_NONE,QSPI_ADDRESS_1_LINE,QSPI_ADDRESS_2_LINE,QSPI_ADDRESS_4_LINE
 * @param   addressSize		地址長度;QSPI_ADDRESS_8_BITS,QSPI_ADDRESS_16_BITS,QSPI_ADDRESS_24_BITS,QSPI_ADDRESS_32_BITS
 * @param   dataMode		數據模式; QSPI_DATA_NONE,QSPI_DATA_1_LINE,QSPI_DATA_2_LINE,QSPI_DATA_4_LINE
 *
 * @return  void
 */
uint32_t QSPI_Send_CMD(uint32_t instruction, uint32_t address, uint32_t dummyCycles, uint32_t instructionMode, uint32_t addressMode, uint32_t addressSize, uint32_t dataMode)
{
    QSPI_CommandTypeDef s_command;

    s_command.Instruction = instruction;                     // 指令
    s_command.Address = address;                             // 地址
    s_command.DummyCycles = dummyCycles;                     // 設定空指令週期數
    s_command.InstructionMode = instructionMode;             // 指令模式
    s_command.AddressMode = addressMode;                     // 地址模式
    s_command.AddressSize = addressSize;                     // 地址長度
    s_command.DataMode = dataMode;                           // 數據模式
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;           // 每次都發送指令
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; // 無交替位元組
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;               // 關閉DDR模式
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;

    return HAL_QSPI_Command(&QSPI_W25Q, &s_command, 5000);
}

uint8_t QSPI_Receive(const uint8_t *buf, uint32_t datalen)
{
    QSPI_W25Q.Instance->DLR = datalen - 1; // Configuration data length
    if (HAL_QSPI_Receive(&QSPI_W25Q, (uint8_t *)buf, 5000) == HAL_OK)
        return 0; // receive data

    else
        return 1;
}

uint8_t QSPI_Transmit(const int8_t *buf, uint32_t datalen)
{
    QSPI_W25Q.Instance->DLR = datalen - 1; // Configuration data length
    if (HAL_QSPI_Transmit(&QSPI_W25Q, (uint8_t *)buf, 5000) == HAL_OK)
        return 0; // send data

    else
        return 1;
}

static void prv_spi_flash_write_enable(void)
{
    QSPI_Send_CMD(QSPI_FLASH_WriteEnable, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, QSPI_DATA_NONE);
}

static void prv_spi_flash_wait_write_end(void)
{
    //! must add the timeout mechanism (*important)

    uint8_t status = 0;

    /* Loop as long as the memory is busy with a write cycle */
    do
    {
        /* Send a dummy byte to generate the clock needed by the FLASH
        and put the value of the status register in status variable */
        QSPI_Send_CMD(QSPI_FLASH_ReadStatusReg, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, QSPI_DATA_1_LINE);
        QSPI_Receive(&status, 1);

    } while ((status & QSPI_FLASH_WIP_FLAG) == SET); /* Write in progress */
}

static int prv_spi_flash_write_page(const uint8_t *buf, uint32_t addr, int32_t len)
{
    int ret = 0;

    if (0 == len)
    {
        return 0;
    }

    prv_spi_flash_write_enable(); // Write enable

    QSPI_Send_CMD(QSPI_FLASH_PageProgram, addr, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);
    QSPI_Transmit((const int8_t *)buf, len);

    prv_spi_flash_wait_write_end(); // Waiting for Writing to End

    return ret;
}

int prv_spi_flash_erase_sector(uint32_t addr)
{
    // printf("fe:%x\r\n",addr);			//監視flash擦除情況,測試用
    int ret = 0;
    prv_spi_flash_write_enable(); // Write enable
    prv_spi_flash_wait_write_end();

    ret = QSPI_Send_CMD(QSPI_FLASH_SectorErase, addr, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_NONE);

    prv_spi_flash_wait_write_end(); // Waiting for Writing to End

    return ret;
}

void hal_spi_flash_config(void)
{
  #define USE_QSPI_BANK_2 (1)

    QSPI_W25Q.Instance = QUADSPI;
    QSPI_W25Q.Init.ClockPrescaler = 2;
    QSPI_W25Q.Init.FifoThreshold = 4;
    QSPI_W25Q.Init.FlashSize = POSITION_VAL(0x1000000) - 1;
    QSPI_W25Q.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_4_CYCLE;
    QSPI_W25Q.Init.ClockMode = QSPI_CLOCK_MODE_0;
    QSPI_W25Q.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  
  #if(USE_QSPI_BANK_2 == 0)  
    QSPI_W25Q.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    QSPI_W25Q.Init.FlashID = QSPI_FLASH_ID_1;
  #else //* bank2
    QSPI_W25Q.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
    QSPI_W25Q.Init.FlashID = QSPI_FLASH_ID_2;
  #endif 
          
    if (HAL_QSPI_Init(&QSPI_W25Q) != HAL_OK)
    {
        Error_Handler();
    }
}

int hal_spi_flash_erase(uint32_t addr, int32_t len)
{
    uint32_t begin;
    uint32_t end;
    int i;

    if (len < 0 || addr > QSPI_FLASH_TOTAL_SIZE || addr + len > QSPI_FLASH_TOTAL_SIZE)
    {
        return -1;
    }

    begin = addr / QSPI_FLASH_SECTOR * QSPI_FLASH_SECTOR;
    end = (addr + len - 1) / QSPI_FLASH_SECTOR * QSPI_FLASH_SECTOR;

    for (i = begin; i <= end; i += QSPI_FLASH_SECTOR)
    {
        if (prv_spi_flash_erase_sector(i) == -1)
        {
            return -1;
        }
    }

    return 0;
}

int hal_spi_flash_write(const void *buf, int32_t len, uint32_t *location)
{
    const uint8_t *pbuf = (const uint8_t *)buf;
    int page_cnt = 0;
    int remain_cnt = 0;
    int temp = 0;
    uint32_t loc_addr;
    uint8_t addr = 0;
    uint8_t count = 0;
    int i;
    int ret = 0;

    if (NULL == pbuf || NULL == location || len < 0 || *location > QSPI_FLASH_TOTAL_SIZE || len + *location > QSPI_FLASH_TOTAL_SIZE)
    {
        return -1;
    }

    loc_addr = *location;
    addr = loc_addr % QSPI_FLASH_PAGESIZE;
    count = QSPI_FLASH_PAGESIZE - addr;
    page_cnt = len / QSPI_FLASH_PAGESIZE;
    remain_cnt = len % QSPI_FLASH_PAGESIZE;

    if (addr == 0) /* addr is aligned to SPI_FLASH_PAGESIZE */
    {
        if (page_cnt == 0) /* len < SPI_FLASH_PAGESIZE */
        {
            ret = prv_spi_flash_write_page(pbuf, loc_addr, len);
            CHECK_RET_RETURN(ret);
        }
        else /* len > SPI_FLASH_PAGESIZE */
        {
            for (i = 0; i < page_cnt; ++i)
            {
                ret = prv_spi_flash_write_page(pbuf + i * QSPI_FLASH_PAGESIZE, loc_addr, QSPI_FLASH_PAGESIZE);
                CHECK_RET_RETURN(ret);
                loc_addr += QSPI_FLASH_PAGESIZE;
            }

            ret = prv_spi_flash_write_page(pbuf + page_cnt * QSPI_FLASH_PAGESIZE, loc_addr, remain_cnt);
            CHECK_RET_RETURN(ret);
        }
    }
    else /* addr is not aligned to SPI_FLASH_PAGESIZE */
    {
        if (page_cnt == 0) /* len < SPI_FLASH_PAGESIZE */
        {
            if (remain_cnt > count) /* (len + loc_addr) > SPI_FLASH_PAGESIZE */
            {
                temp = remain_cnt - count;

                ret = prv_spi_flash_write_page(pbuf, loc_addr, count);
                CHECK_RET_RETURN(ret);

                ret = prv_spi_flash_write_page(pbuf + count, loc_addr + count, temp);
                CHECK_RET_RETURN(ret);
            }
            else
            {
                ret = prv_spi_flash_write_page(pbuf, loc_addr, len);
                CHECK_RET_RETURN(ret);
            }
        }
        else /* len > SPI_FLASH_PAGESIZE */
        {
            len -= count;
            page_cnt = len / QSPI_FLASH_PAGESIZE;
            remain_cnt = len % QSPI_FLASH_PAGESIZE;

            ret = prv_spi_flash_write_page(pbuf, loc_addr, count);
            CHECK_RET_RETURN(ret);
            loc_addr += count;

            for (i = 0; i < page_cnt; ++i)
            {
                ret = prv_spi_flash_write_page(pbuf + count + i * QSPI_FLASH_PAGESIZE, loc_addr, QSPI_FLASH_PAGESIZE);
                CHECK_RET_RETURN(ret);
                loc_addr += QSPI_FLASH_PAGESIZE;
            }

            if (remain_cnt != 0)
            {
                ret = prv_spi_flash_write_page(pbuf + count + page_cnt * QSPI_FLASH_PAGESIZE, loc_addr, remain_cnt);
                CHECK_RET_RETURN(ret);
            }
        }
    }

    *location += len;
    return ret;
}

int hal_spi_flash_erase_write(const void *buf, int32_t len, uint32_t location)
{
    int ret = 0;

    ret = hal_spi_flash_erase(location, len);
    CHECK_RET_RETURN(ret);
    ret = hal_spi_flash_write(buf, len, &location);

    return ret;
}

int hal_spi_flash_read(void *buf, int32_t len, uint32_t location)
{
    int ret = 0;
    uint8_t *pbuf = (uint8_t *)buf;

    if (NULL == pbuf || len < 0 || location > QSPI_FLASH_TOTAL_SIZE || len + location > QSPI_FLASH_TOTAL_SIZE)
    {
        return -1;
    }

    QSPI_Send_CMD(QSPI_FLASH_FastReadData, location, 8, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);
    QSPI_Receive(buf, len);

    return ret;
}

int hal_spi_flash_get_id(void)
{
    QSPI_CommandTypeDef s_command;
    uint8_t temp[3];
    uint32_t deviceid;

    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction = QSPI_FLASH_JedecDeviceID;
    s_command.AddressMode = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize = QSPI_ADDRESS_24_BITS;
    s_command.DataMode = QSPI_DATA_1_LINE;
    s_command.AddressMode = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DummyCycles = 0;
    s_command.NbData = 3;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    HAL_QSPI_Command(&QSPI_W25Q, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

    QSPI_Receive(temp, 3);
    deviceid = (temp[1] << 8) | (temp[0] << 16) | (temp[2]);
    return deviceid;
}
