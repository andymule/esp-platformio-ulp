#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include "esp_sleep.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/sens_reg.h"
#include "soc/soc.h"
#include "driver/rtc_io.h"
#include "esp32/ulp.h"
#include "sdkconfig.h"
#include "ulp_main.h"
#include "math.h"


extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");

#define rtcout(x) rtc_gpio_init(x); \
    rtc_gpio_set_direction(x, RTC_GPIO_MODE_OUTPUT_ONLY); \
    rtc_gpio_set_level(x, 0);

#define rtcouthi(x) rtc_gpio_init(x); \
    rtc_gpio_set_direction(x, RTC_GPIO_MODE_OUTPUT_ONLY); \
    rtc_gpio_set_level(x, 1);

#define rtcin(x) rtc_gpio_init(x); \
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
const gpio_num_t GPIO_CS = GPIO_NUM_4;    
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
// 2->12
// 4->10
// 12->15
// 13->4
// 14->16
// 15->13
// 26->7
// 27->17
// 32->9
// 33->8
// 34->4

static void init_ulp_program()
{
    esp_err_t err = ulp_load_binary(0, ulp_main_bin_start,(ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));         
    ESP_ERROR_CHECK(err);

    // rtcout(GPIO_MOSI)
    // rtcout(GPIO_SCLK)
    // rtcout(GPIO_CS) // low to select/enable
    // rtcouthi(GPIO_RST) // low to reset 
    // rtcout(GPIO_DC) // hi data, lo command (only 4 pin spi) // unused since BS1 high? 
    // // rtcout(GPIO_DC)// BSI // select SPI: Low if 4pin, HI is 3 pin   
    // // BS1 TIED HIGH according to schematic
    // rtcin(GPIO_MISO) // unused? should tie high or low?
    // rtcin(GPIO_BUSY)

    /* Set ULP wake up period to 2s */
    ulp_set_wakeup_period(0, 2*1000*1000);
}

RTC_DATA_ATTR int wake_count;
void RTC_IRAM_ATTR  esp_wake_deep_sleep()
{
    esp_default_wake_deep_sleep();
    static RTC_RODATA_ATTR const char fmt_str[] = "Wake count %d\n";
    ets_printf(fmt_str, wake_count++);
}

void app_main()
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    if (cause != ESP_SLEEP_WAKEUP_ULP) {
        printf("Not ULP wakeup, initializing ULP\n");
    } else {
        printf("Deep sleep wakeup\n");
    }
    init_ulp_program();
    printf("Entering deep sleep\n\n");
    esp_deep_sleep_disable_rom_logging(); // suppress boot messages
    /* Start the ULP program */
    ESP_ERROR_CHECK( ulp_run((&ulp_entry - RTC_SLOW_MEM) / sizeof(uint32_t)));
    ESP_ERROR_CHECK( esp_sleep_enable_ulp_wakeup() );
    esp_deep_sleep_start();
}