/* ULP Example: using ADC in deep sleep
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
   This file contains assembly code which runs on the ULP.
   ULP wakes up to run this code at a certain period, determined by the values
   in SENS_ULP_CP_SLEEP_CYCx_REG registers. On each wake up, the program
   measures input voltage on the given ADC channel 'adc_oversampling_factor'
   times. Measurements are accumulated and average value is calculated.
   Average value is compared to the two thresholds: 'low_thr' and 'high_thr'.
   If the value is less than 'low_thr' or more than 'high_thr', ULP wakes up
   the chip from deep sleep.
*/

#include <ulp_c.h>

unsigned init_str[30] = {0x02,0x02,0x01,0x11,0x12,0x12,0x22,0x22,0x66,0x69,0x69,0x59,0x58,0x99,0x99,0x88,0x00,0x00,0x00,0x00,0xF8,0xB4,0x13,0x51,0x35,0x51,0x51,0x19,0x01,0x00};

unsigned faker[30];
// unsigned init_partial[30] = {0x10,0x18,0x18,0x08,0x18,0x18,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x13,0x14,0x44,0x12,0x00,0x00,0x00,0x00,0x00,0x00}

void entry()
{
    unsigned  j=0; 
    unsigned  i;
    for (i=0; i<30; i = i+1)
    {
        faker[i] = init_str[i]; 
    }
}
