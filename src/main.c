/**
 * @file
 * @author Said Alvarado-Marin <said-alexander.alvarado-marin@inria.fr>
 * @author Arnaud Taffanel <arnaud@bitcraze.io>
 * @brief Lighthouse-16 deck main program
 *
 * Lighthouse-16 deck firmware. Intended to be run on an RP2350
 *
 * @date 2024,2025
 *
 * @copyright Inria, 2024
 * @copyright Bitcraze AB, 2025
 *
 */
#include "hardware/pio.h"
#include "pico/time.h"
#include "lh2/lh2.h"
#include "pico/stdlib.h"
// #include "pico/cyw43_arch.h"
#include <stdio.h>
#include <stdlib.h>

#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"

//=========================== defines ==========================================

#define LH2_0_DATA_PIN  13
#define LH2_0_ENV_PIN   12
#define LH2_1_DATA_PIN  17
#define LH2_1_ENV_PIN   16
#define LH2_2_DATA_PIN  1
#define LH2_2_ENV_PIN   0
#define LH2_3_DATA_PIN  29
#define LH2_3_ENV_PIN   28
#define TIMER_DELAY_US 100000


//=========================== variables ========================================

// is nedeed so the variable is accesible to both cores [1]
db_lh2_t        _lh2_0;
db_lh2_t        _lh2_1;
db_lh2_t        _lh2_2;
db_lh2_t        _lh2_3;
absolute_time_t timer_0;
bool            clk_conf_OK;

uint8_t sensor_0 = 0;
uint8_t sensor_1 = 1;
uint8_t sensor_2 = 2;
uint8_t sensor_3 = 3;

//=========================== prototypes ========================================

void core1_entry();

//=========================== main core #0 =============================================

int main() {
    // configure the clock for 128MHz
    clk_conf_OK = set_sys_clock_khz(128000, true);

    // init the USB UART
    stdio_init_all();
    sleep_ms(3000);
    printf("Start code\n");

    // LH2 config, before starting the second core
    db_lh2_init(&_lh2_0, sensor_0, LH2_0_DATA_PIN, LH2_0_ENV_PIN);
    db_lh2_init(&_lh2_1, sensor_1, LH2_1_DATA_PIN, LH2_1_ENV_PIN);

    // Launch the second core
    multicore_launch_core1(core1_entry);

    timer_0 = get_absolute_time();

    while (true) {

        // the location function has to be running all the time
        db_lh2_process_location(&_lh2_0);
        db_lh2_process_location(&_lh2_1);

        if (absolute_time_diff_us(timer_0, get_absolute_time()) > TIMER_DELAY_US) {

            printf("sen_0 (%d-%d %d-%d %d-%d %d-%d)   \tsen_1 (%d-%d %d-%d %d-%d %d-%d)   \tsen_2 (%d-%d %d-%d %d-%d %d-%d)   \tsen_3 (%d-%d %d-%d %d-%d %d-%d)\n",
                   _lh2_0.locations[0][0].selected_polynomial, _lh2_0.locations[0][0].lfsr_location, _lh2_0.locations[1][0].selected_polynomial, _lh2_0.locations[1][0].lfsr_location,
                   _lh2_0.locations[0][1].selected_polynomial, _lh2_0.locations[0][1].lfsr_location, _lh2_0.locations[1][1].selected_polynomial, _lh2_0.locations[1][1].lfsr_location,
                   _lh2_1.locations[0][0].selected_polynomial, _lh2_1.locations[0][0].lfsr_location, _lh2_1.locations[1][0].selected_polynomial, _lh2_1.locations[1][0].lfsr_location,
                   _lh2_1.locations[0][1].selected_polynomial, _lh2_1.locations[0][1].lfsr_location, _lh2_1.locations[1][1].selected_polynomial, _lh2_1.locations[1][1].lfsr_location,
                   _lh2_2.locations[0][0].selected_polynomial, _lh2_2.locations[0][0].lfsr_location, _lh2_2.locations[1][0].selected_polynomial, _lh2_2.locations[1][0].lfsr_location,
                   _lh2_2.locations[0][1].selected_polynomial, _lh2_2.locations[0][1].lfsr_location, _lh2_2.locations[1][1].selected_polynomial, _lh2_2.locations[1][1].lfsr_location,
                   _lh2_3.locations[0][0].selected_polynomial, _lh2_3.locations[0][0].lfsr_location, _lh2_3.locations[1][0].selected_polynomial, _lh2_3.locations[1][0].lfsr_location,
                   _lh2_3.locations[0][1].selected_polynomial, _lh2_3.locations[0][1].lfsr_location, _lh2_3.locations[1][1].selected_polynomial, _lh2_3.locations[1][1].lfsr_location);
            timer_0 = get_absolute_time();
        }
    }
}

//=========================== main core #0 =============================================

void core1_entry() {

    db_lh2_init(&_lh2_2, sensor_2, LH2_2_DATA_PIN, LH2_2_ENV_PIN);
    db_lh2_init(&_lh2_3, sensor_3, LH2_3_DATA_PIN, LH2_3_ENV_PIN);

    while (true) {
        db_lh2_process_location(&_lh2_2);
        db_lh2_process_location(&_lh2_3);
    }
}

//=========================== references ========================================
