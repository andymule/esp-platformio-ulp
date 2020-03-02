
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/soc_ulp.h"
#include "soc/sens_reg.h"
#include "stack.S"

    // these are from temp exmaple, need to replace with my own from python
	.set	MS5611_ADDR,             0x77 // MS5611
	.set	CMD_RESET,               0xFF // ADC reset command
	.set	CMD_ADC_READ,            0x00 // ADC read command
	.set	CMD_ADC_D1_256,          0x40 // ADC OSR=256
	.set	CMD_ADC_D1_512,          0x42 // ADC OSR=512
	.set	CMD_ADC_D1_1024,         0x44 // ADC OSR=1024
	.set	CMD_ADC_D1_2048,         0x46 // ADC OSR=2048
	.set	CMD_ADC_D1_4096,         0x48 // ADC OSR=4096
	.set	CMD_ADC_D2_256,          0x50 // ADC OSR=256
	.set	CMD_ADC_D2_512,          0x52 // ADC OSR=512
	.set	CMD_ADC_D2_1024,         0x54 // ADC OSR=1024
	.set	CMD_ADC_D2_2048,         0x56 // ADC OSR=2048
	.set	CMD_ADC_D2_4096,         0x58 // ADC OSR=4096
	.set	CMD_PROM_RD,             0xA0 // Prom read command
	.set	PROM_NB,                 0x08 // PROM lenth

	.set EPD_WIDTH, 128
	.set EPD_HEIGHT, 296

	// Display commands
	.set  DRIVER_OUTPUT_CONTROL, 0x01
	.set  BOOSTER_SOFT_START_CONTROL, 0x0C
	// GATE_SCAN_START_POSITION            , 0x0F
	.set  DEEP_SLEEP_MODE, 0x10
	.set  DATA_ENTRY_MODE_SETTING, 0x11
	// SW_RESET                            , 0x12
	// TEMPERATURE_SENSOR_CONTROL          , 0x1A
	.set  MASTER_ACTIVATION, 0x20
	// DISPLAY_UPDATE_CONTROL_1            , 0x21
	.set  DISPLAY_UPDATE_CONTROL_2, 0x22
	.set  WRITE_RAM, 0x24
	.set  WRITE_VCOM_REGISTER, 0x2C
	.set  WRITE_LUT_REGISTER, 0x32
	.set  SET_DUMMY_LINE_PERIOD, 0x3A
	.set  SET_GATE_TIME, 0x3B
	// BORDER_WAVEFORM_CONTROL             , 0x3C
	.set  SET_RAM_X_ADDRESS_START_END_POSITION, 0x44
	.set  SET_RAM_Y_ADDRESS_START_END_POSITION, 0x45
	.set  SET_RAM_X_ADDRESS_COUNTER, 0x4E
	.set  SET_RAM_Y_ADDRESS_COUNTER, 0x4F
	.set  TERMINATE_FRAME_READ_WRITE, 0xFF

#ifdef SPI_BIT16 				/* if define SPI_BIT16 write and read 16bit */
	.set bit_mask, 	0x8000
	.set bit_len, 	0x10
#else 							/* default spi write and read 8bit */
	.set bit_mask, 	0x80
	.set bit_len, 	0x08
#endif

	.set SPI_MODE_1,	1		/* Mode_1, Clock Polarity is 0 and Clock Phase is 0 */
	.set SPI_MODE_2,	2		/* Mode_2, Clock Polarity is 0 and Clock Phase is 1 */
	.set SPI_MODE_3,	3		/* Mode_3, Clock Polarity is 1 and Clock Phase is 0 */
	.set SPI_MODE_4,	4		/* Mode_4, Clock Polarity is 1 and Clock Phase is 1 */
	.set SPI_MASTER,	0		/* SPI Master */
	.set SPI_SLAVE,		1		/* SPI Slave */
	.set SPI_MODE_SET,	SPI_MODE_1
	.set SPI_TYPE_SET,	SPI_MASTER

	.set pMOSI, 7
	.set pSCLK, 12
	.set pCS, 17
	.set pMISO, 15
	.set pDC, 4
	.set pRST, 16
	.set pBUSY, 13

	

/*
; // for to test w bus pirate				   	//exposed GPIO w RTC:
; const gpio_num_t GPIO_MOSI = GPIO_NUM_0; 	// 0->11
; const gpio_num_t GPIO_SCLK = GPIO_NUM_2; 	// 2->12
; const gpio_num_t GPIO_CS = GPIO_NUM_4;   	// 4->10
; const gpio_num_t GPIO_MISO = GPIO_NUM_12;	// 12->15
; const gpio_num_t GPIO_DC = GPIO_NUM_13;  	// 13->4
; const gpio_num_t GPIO_RST = GPIO_NUM_14; 	// 14->16
; const gpio_num_t GPIO_BUSY = GPIO_NUM_15;	// 15->13

; .macro read_SCL // Return current level of SCL line, 0 or 1
; READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S + 9, 1) // RTC_GPIO_9 == GPIO_32
; .endm
*/

/* Define variables, which go into .bss section (zero-initialized data) */
.bss
.global stack
stack:
	.skip 100
	.global stackEnd
stackEnd:
	.long 0

	.global spi_mode
spi_mode:
	.long	0

reg_addr:
	.long 0
	.global counter				/* PROM read lenth counter */
counter:
	.long 0
	.global prom_table
prom_table:
	.skip 32 					/* 128bit, 8 x 4(byte) */

	.global addr_pointer 		/* read PROM data pointer */
addr_pointer:
	.long 0
	.global temp
temp:
	.long 0

	.global swap
swap:      .long 0

.data
readcnt: .long 0

	/* Code goes into .text section */
	.text

.macro spi_delay // called before and after a send, so half it?
	//wait 17  // 8000000 cycles/second... this implies 800khz transfer?
	wait 0xFF  // 8563304 
.endm

.macro read_MISO 
	READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S + pMISO, 1) 
.endm

.macro clear_SCLK
	WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + pSCLK, 1, 1)
.endm
.macro set_SCLK
	WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + pSCLK, 1, 1)
.endm

.macro clear_MOSI
	WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + pMOSI, 1, 1)
.endm
.macro set_MOSI
	WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + pMOSI, 1, 1)
.endm

.macro clear_CS 
	WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + pCS, 1, 1)
.endm
.macro set_CS 
	WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + pCS, 1, 1)
.endm


	.global entry
entry:
	clear addr_pointer
	clear counter
	clear reg_addr

	// init
	set_CS 	
	clear_MOSI
	clear_SCLK
	clear_CS 				/* enable cs bus */ //jump takes 4 cycles
	
	//debug
	//move r2, CMD_RESET 			/* load CMD_RESET into reg to be written */ //move takes 6 cycles
	//psr
	//jump SPI_Write_Byte 		/* send the command */
	
	move r0, 0 // int index = 0
loopInit:
    move r2, init_str   // r2 = init_str[0]
	add r2, r2, r0
    ld r2, r2, 0      // r2 = init_str[index]
	move r1, readcnt // save val of counter
	st	r0, R1, 0
	psr
	jump SPI_Write_Byte 		/* send the command */
	move r1, readcnt // save val of counter
    ld	r0, R1, 0
	add r0, r0, 1     // this increments address by 2 words (8 bytes)
	jumpr loopInit, 30, lt
	set_CS 			/* disable CS */

	move r2, 20
	psr
	jump waitMs
	halt

	.global init_param
init_param:
	
	ret

        .global waitMs
waitMs:
	wait 0x2173 // = decimal 8563 // // 8563304 MHZ // used to be 8000
	sub r2,r2, 0x01 				/* Wait for r2 milliseconds */
	jump doneWaitMs,eq
	jump waitMs
doneWaitMs:
	ret

	.global exit
exit:
	halt
	.global wake_up
wake_up:
	/* Check if the SoC can be woken up */
	READ_RTC_REG(RTC_CNTL_DIAG0_REG, 19, 1)
	and r0, r0, 0x01
	jump exit, eq
	//jump wake_up, eq
	/* Wake up the SoC and stop ULP program */
	wake
	/* Stop the wakeup timer so it does not restart ULP */
	WRITE_RTC_FIELD(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN, 0)
	halt

	.global SPI_Init
SPI_Init:
	set_CS 						/* disable CS bus */
gpio_init1:
	clear_MOSI
	clear_SCLK
	ret

	.global CS_Disable 			/* CS high level signal disable */
CS_Disable:
	set_CS
	ret

	.global CS_Enable 			/* CS low level signal enable */
CS_Enable:
	clear_CS
	ret

	.global	SPI_Write_Byte 		/* r2 save the data to be sent out */
SPI_Write_Byte:
write_mode_1:					/* Clock Polarity is 0 and Clock Phase is 0 */
	stage_rst
write_loop1:
	clear_SCLK
	and r0, r2, bit_mask
	lsh r2, r2, 0x01
	jumpr loop1_bit0, 0x01, lt
	set_MOSI
	jump loop1_bit1
loop1_bit0:
	clear_MOSI
loop1_bit1:
	spi_delay
	set_SCLK
	spi_delay
	stage_inc 0x01
	jumps write_loop1, bit_len, lt
	clear_SCLK
	jump spi_write_byte_end

spi_write_byte_end:
	clear_MOSI
	ret

	.global SPI_Burst_Write
SPI_Burst_Write:
	clear_CS
	spi_delay
	ret

init_str: //30 char
.long 0x2
.long 0x2
.long 0x1
.long 0x11
.long 0x12
.long 0x12
.long 0x22
.long 0x22
.long 0x66
.long 0x69
.long 0x69
.long 0x59
.long 0x58
.long 0x99
.long 0x99
.long 0x88
.long 0x0
.long 0x0
.long 0x0
.long 0x0
.long 0xf8
.long 0xb4
.long 0x13
.long 0x51
.long 0x35
.long 0x51
.long 0x51
.long 0x19
.long 0x1
.long 0x0