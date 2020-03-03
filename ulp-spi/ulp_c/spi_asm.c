#include <ulp_c.h>

#define bit_mask 0x80
#define bit_len 0x08
#define clockWaitSPIHZ 255 // cycles between SPI clocks

#define pMOSI 7
#define pSCLK 12
#define pCS 17
#define pMISO 15
#define pDC 4
#define pRST 16
#define pBUSY 13

#define read_MISO READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S + pMISO, 1)
#define disable_RST WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + pRST, 1, 1)
#define enable_RST WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + pRST, 1, 1)
#define clear_SCLK WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + pSCLK, 1, 1)
#define set_SCLK WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + pSCLK, 1, 1)
#define clear_MOSI WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + pMOSI, 1, 1)
#define set_MOSI WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + pMOSI, 1, 1)
#define disable_CS WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + pCS, 1, 1)
#define enable_CS WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + pCS, 1, 1)

unsigned swap[30];
const unsigned int init_str_len = 30;
const unsigned init_str[30] = {0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22, 0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51, 0x35, 0x51, 0x51, 0x19, 0x01, 0x00};
const unsigned init_partial_str[30] = {0x10,0x18,0x18,0x08,0x18,0x18,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x13,0x14,0x44,0x12,0x00,0x00,0x00,0x00,0x00,0x00};

// SPI mode 1 hard coded
void entry()
{
    unsigned loopcnt = 0;
    unsigned datasize = 0;

    enable_RST;
    wait(65535); // 1/129 seconds?
    disable_RST;
    wait(65535);

SPI_Send:
    disable_CS; /* disable CS bus */
    clear_MOSI;
    clear_SCLK;

    enable_CS;

    loopcnt = 0;
    datasize = init_str_len;
    while (loopcnt < datasize)
    {
        // unsigned mydatabyte = init_str[loopcnt];
        unsigned mydatabyte = swap[loopcnt];
        unsigned thisbit;
        unsigned bitsToSend = 8;
        loopcnt += 1;
        while (bitsToSend > 0)
        {
            bitsToSend -= 1;
            thisbit = mydatabyte & bit_mask;
            mydatabyte = mydatabyte << 1;
            if (thisbit < 1)
            {
                clear_MOSI;
            }
            else
            {
                set_MOSI;
            }
            clear_SCLK;
            wait(clockWaitSPIHZ);
            set_SCLK;
            wait(clockWaitSPIHZ);
        }
    }

    clear_MOSI;
    halt;
}
