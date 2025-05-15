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
#include <string.h>

#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"

//=========================== defines ==========================================

#define LH2_0_DATA_PIN  13
#define LH2_0_ENV_PIN   12

#define LH2_1_DATA_PIN  18
#define LH2_1_ENV_PIN   17

#define LH2_2_DATA_PIN  1
#define LH2_2_ENV_PIN   0

#define LH2_3_DATA_PIN  29
#define LH2_3_ENV_PIN   28

#define TIMER_DELAY_US 100000

#define LED_RED_PIN  22
#define LED_YELLOW_PIN 21  
#define LED_GREEN_PIN  20

#define SYNC_PERIOD_MS 500


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

queue_t measurements_queue;

struct measurement_frame {
    uint32_t sensor_id:2;
    uint32_t polynomial_id:6;
    uint32_t dummy:24;
    uint32_t lfsr_location;
    uint32_t timestamp;
} __attribute__((packed));
typedef struct measurement_frame measurement_frame_t;

//=========================== prototypes ========================================

void core1_entry();

//=========================== main core #0 =============================================

int main() {
    absolute_time_t last_sync = 0;
    static char sync_packet[12];
    static measurement_frame_t packet;

    memset(sync_packet, 0xff, sizeof(sync_packet));

    // configure the clock for 128MHz
    clk_conf_OK = set_sys_clock_khz(128000, true);

    // init the USB UART
    stdio_init_all();

    // init the LEDs
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_init(LED_YELLOW_PIN);
    gpio_set_dir(LED_YELLOW_PIN, GPIO_OUT);
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_RED_PIN, 1);               // 1 is OFF
    gpio_put(LED_YELLOW_PIN, 1);
    gpio_put(LED_GREEN_PIN, 1);

    queue_init(&measurements_queue, sizeof(struct lh2_measurement), 10);

    // LH2 config, before starting the second core
    db_lh2_init(&_lh2_0, sensor_0, LH2_0_DATA_PIN, LH2_0_ENV_PIN);
    db_lh2_init(&_lh2_1, sensor_1, LH2_1_DATA_PIN, LH2_1_ENV_PIN);

    // Launch the second core
    multicore_launch_core1(core1_entry);

    timer_0 = get_absolute_time();

    while (true) {

        // the location function has to be running all the time
        db_lh2_process_location(&_lh2_0, &measurements_queue);
        db_lh2_process_location(&_lh2_1, &measurements_queue);

        // Receive data from the queue and print them
        struct lh2_measurement measurement;
        while  (queue_try_remove(&measurements_queue, &measurement)) {
            memset(&packet, 0, sizeof(packet));
            packet.sensor_id = measurement.sensor;
            packet.polynomial_id = measurement.selected_polynomial;
            packet.lfsr_location = measurement.lfsr_location;
            packet.timestamp = measurement.timestamp;
            stdio_put_string((char *)&packet, sizeof(packet), false, false);
        }

        if (absolute_time_diff_us(last_sync, get_absolute_time()) > SYNC_PERIOD_MS * 1000) {
            last_sync = get_absolute_time();
            stdio_put_string(sync_packet, sizeof(sync_packet), false, false);
        }
    }
}

//=========================== main core #0 =============================================

void core1_entry() {

    db_lh2_init(&_lh2_2, sensor_2, LH2_2_DATA_PIN, LH2_2_ENV_PIN);
    db_lh2_init(&_lh2_3, sensor_3, LH2_3_DATA_PIN, LH2_3_ENV_PIN);

    while (true) {
        db_lh2_process_location(&_lh2_2, &measurements_queue);
        db_lh2_process_location(&_lh2_3, &measurements_queue);
    }
}

//=========================== references ========================================
