#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/sens_reg.h"
#include "soc/soc.h"
#include "soc/rtc.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"
#include "esp32/ulp.h"
#include "sdkconfig.h"
#include "ulp_main.h"
#include "math.h"

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

#define rtcout(x)                                         \
    rtc_gpio_init(x);                                     \
    rtc_gpio_set_direction(x, RTC_GPIO_MODE_OUTPUT_ONLY); \
    rtc_gpio_set_level(x, 0);

#define rtcouthi(x)                                       \
    rtc_gpio_init(x);                                     \
    rtc_gpio_set_direction(x, RTC_GPIO_MODE_OUTPUT_ONLY); \
    rtc_gpio_set_level(x, 1);

#define rtcin(x)                                         \
    rtc_gpio_init(x);                                    \
    rtc_gpio_set_direction(x, RTC_GPIO_MODE_INPUT_ONLY); \
    rtc_gpio_pullup_en(x);

// // for lilygo T5 v2.2
// const gpio_num_t GPIO_MOSI = GPIO_NUM_23;   // nortc
// const gpio_num_t GPIO_SCLK = GPIO_NUM_18;   // nortc
// const gpio_num_t GPIO_CS = GPIO_NUM_5;      // nortc
// const gpio_num_t GPIO_MISO = GPIO_NUM_2;    //RTC12    unused ?
// const gpio_num_t GPIO_DC = GPIO_NUM_19;     // nortc   unused ?
// const gpio_num_t GPIO_RST = GPIO_NUM_12;    //RTC15
// const gpio_num_t GPIO_BUSY = GPIO_NUM_4;    //RTC10

// for to test w bus pirate
//worked on arduino gpio: SCLK = 2, MISO = 12, MOSI = 26, SS = 4,
const gpio_num_t GPIO_MOSI = GPIO_NUM_26;
const gpio_num_t GPIO_SCLK = GPIO_NUM_2;
const gpio_num_t GPIO_CS = GPIO_NUM_27;
const gpio_num_t GPIO_MISO = GPIO_NUM_12;
const gpio_num_t GPIO_DC = GPIO_NUM_13;
const gpio_num_t GPIO_RST = GPIO_NUM_14;
const gpio_num_t GPIO_BUSY = GPIO_NUM_15;

// as defined in original example
// const gpio_num_t GPIO_CS = GPIO_NUM_25;
// const gpio_num_t GPIO_MOSI = GPIO_NUM_26;
// const gpio_num_t GPIO_SCLK = GPIO_NUM_27;
// const gpio_num_t GPIO_MISO = GPIO_NUM_4;

//exposed GPIO w RTC:
// 0->11 // DONT USE used to boot into flash mode
// 2->12   x
// 4->10     // had trouble here too, idk
// 12->15  x
// 13->4
// 14->16
// 15->13
// 26->7   x
// 27->17  x  //last used
// 32->9
// 33->8
// 34->4
//pins 34-39 can be used as input only while all the other pins can be used as input or output
//esp32 manual: 32-33 output only?

// The ULP co-processor puts itself into sleep mode by executing the HALT instruction. This also triggers the ULPtimer to start counting RTC_SLOW_CLK ticks which, by default, originate from an internal 150 kHz RC oscillator.Once the timer expires, the ULP co-processor is powered up and runs a program with the program counter (PC)which is stored in register SENS_PC_INIT. The relationship between the described signals and registers is shownin Figure156.

//Once the timer counts the number of ticks set in the selected SENS_ULP_CP_SLEEP_CYCx_REG register, the ULP coprocessor will power up and start running the program from the entry point set in the call to ulp_run.

static void setup_ulp_pins()
{
    esp_err_t err = ulp_load_binary(0, ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));
    ESP_ERROR_CHECK(err);

    rtcout(GPIO_MOSI)
    rtcout(GPIO_SCLK)
    rtcout(GPIO_CS) // low to select/enable
    rtcouthi(GPIO_RST)  // low to reset
    rtcout(GPIO_DC)     // hi data, lo command (only 4 pin spi) // unused since BS1 high?
    // rtcout(GPIO_DC)// BSI // select SPI: Low if 4pin, HI is 3 pin
    // BS1 TIED HIGH according to schematic
    rtcin(GPIO_MISO) // unused? should tie high or low?
    rtcin(GPIO_BUSY)

     /* Disconnect GPIO12 and GPIO15 to remove current drain through
     * pullup/pulldown resistors.
     * GPIO12 may be pulled high to select flash voltage.
     */
    // rtc_gpio_isolate(GPIO_NUM_12);

    //gpio_set_direction(GPIO_CS, GPIO_MODE_OUTPUT);
    // while (true)
    // {
    //     printf("down\n");
    //     gpio_set_level(GPIO_CS, 0);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    //     printf("up\n");
    //     gpio_set_level(GPIO_CS, 1);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
}

// RTC_DATA_ATTR int wake_count;
// void RTC_IRAM_ATTR  esp_wake_deep_sleep()
// {
//     esp_default_wake_deep_sleep();
//     static RTC_RODATA_ATTR const char fmt_str[] = "Wake count %d\n";
//     ets_printf(fmt_str, wake_count++);
// }

void app_main()
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    if (cause != ESP_SLEEP_WAKEUP_ULP)
    {
        printf("Not ULP wakeup, initializing ULP\n");
    }
    else
    {
        printf("Deep sleep wakeup\n");
    }
    setup_ulp_pins();
    
    // uint32_t rtc_8md256_period = rtc_clk_cal(RTC_CAL_8MD256, 100);
    // uint32_t rtc_fast_freq_hz = 1000000ULL * (1 << RTC_CLK_CAL_FRACT) * 256 / rtc_8md256_period;
    // printf("rtc_8md256_period: %d \n", rtc_8md256_period);
    // printf("rtc_fast_freq_hz: %d \n", rtc_fast_freq_hz); // 8563304 pass to RTC as true speed?
    printf("Entering deep sleep\n\n");
    esp_deep_sleep_disable_rom_logging(); // suppress boot messages
    /* Start the ULP program */
    // memcpy((void*)(&ulp_cmd_full_update), "abcdefghijklmnopqrstuvwxyz", 26);
    // (unsigned char*)(&ulp_cmd_full_update) = "asd";
    ulp_swap = 128;
    printf("Last measurement value: %d\n", ulp_swap & UINT16_MAX);
    /* Set ULP wake up period to 2s */
    int toSec = 1000 * 1000;
    esp_sleep_enable_timer_wakeup(3 * toSec);
    ulp_set_wakeup_period(0, 2 * toSec); //how often to wake ULP
    ESP_ERROR_CHECK(ulp_run((&ulp_entry - RTC_SLOW_MEM) / sizeof(uint32_t)));
    ESP_ERROR_CHECK(esp_sleep_enable_ulp_wakeup());
    esp_deep_sleep_start();
}