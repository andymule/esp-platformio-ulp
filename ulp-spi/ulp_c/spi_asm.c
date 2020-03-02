#include <ulp_c.h>

unsigned swap, counter;
const unsigned int init_str_len = 30;
const unsigned init_str[30] = {0x02,0x02,0x01,0x11,0x12,0x12,0x22,0x22,0x66,0x69,0x69,0x59,0x58,0x99,0x99,0x88,0x00,0x00,0x00,0x00,0xF8,0xB4,0x13,0x51,0x35,0x51,0x51,0x19,0x01,0x00};

#define bit_mask 0x80

#define read_MISO READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S + pMISO, 1) 

#define clear_SCLK WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + pSCLK, 1, 1)
#define set_SCLK WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + pSCLK, 1, 1)

#define clear_MOSI WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + pMOSI, 1, 1)
#define set_MOSI WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + pMOSI, 1, 1)

#define disable_CS WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + pCS, 1, 1)
#define enable_CS WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + pCS, 1, 1)

// unsigned init_partial[30] = {0x10,0x18,0x18,0x08,0x18,0x18,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x13,0x14,0x44,0x12,0x00,0x00,0x00,0x00,0x00,0x00}

// SPI mode 1 hard coded
void entry()
{
    // TODO reset variables
    
SPI_Init:
	disable_CS 						/* disable CS bus */
	clear_MOSI
	clear_SCLK

    enable_CS
    unsigned i = 0;
    // while(i<30)
    while (i<2)
    { 
        unsigned mydatabyte = i;
        ////////////
// SPI_Write_Byte:
    // write_mode_1:					/* Clock Polarity is 0 and Clock Phase is 0 */
        // stage_rst
        // clear_SCLK
    // write_loop1:
        clear_SCLK
        // and r0, r2, bit_mask
        unsigned thisbit = mydatabyte & bit_mask;
        // lsh r2, r2, 1
        thisbit = thisbit << 1;
        jumpr loop1_bit0, 1, lt
        set_MOSI
        jump loop1_bit1
    loop1_bit0:
        clear_MOSI
    loop1_bit1:
        spi_delay
        set_SCLK
        spi_delay
        stage_inc 1
        jumps write_loop1, bit_len, lt
        clear_SCLK
        jump spi_write_byte_end
        //////////////
        // WRITE OUT init_str[i]
        i += 1;
    }

    halt
}
