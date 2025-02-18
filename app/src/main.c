/*
* Copyright (c) 2021 Nordic Semiconductor ASA
* SPDX-License-Identifier: Apache-2.0
*/

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <app_version.h>


LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

// Define the sensor
static const struct device *sensor;

// Declare the timer
struct k_timer my_timer;

// Timer expiry function
void my_timer_expiry_function(struct k_timer *timer_id) {
    int                         ret;
    static struct sensor_value  lastValue;
    struct sensor_value         value;

    LOG_INF("Timer expired - fetching sensor data");

    ret = sensor_sample_fetch(sensor);
    if (ret == -ENODATA){
        LOG_INF("End of csv file.");
        k_timer_stop(&my_timer);
        return;
    }
    else if (ret < 0){
        LOG_ERR("Could not fetch sample (%d)", ret);
        return;
    }

    ret = sensor_channel_get(sensor, SENSOR_CHAN_AMBIENT_TEMP, &value);

    if (ret == -ENODATA){
        LOG_INF("No more sensor data.");
        k_timer_stop(&my_timer);
        return;
    }
    else if (ret < 0){
        LOG_ERR("Could not get sample (%d)", ret);
        return;
    }
    if (lastValue.val1 != value.val1){
        LOG_INF("Sensor Value: %d", value.val1);
    }

    lastValue = value;
}

// Initialize the timer
void init_my_timer(void) {
    k_timer_init(&my_timer, my_timer_expiry_function, NULL);
    LOG_INF("Timer Initialized");
}

// In your function where you start the timer
// start a periodic timer that expires once every CONFIG_MY_TIMER_PERIOD
void start_my_timer(void) {
    k_timer_start(&my_timer,
                  K_SECONDS(CONFIG_EXAMPLE_SENSOR_MY_TIMER_INITIAL_DELAY),
                  K_SECONDS(CONFIG_EXAMPLE_SENSOR_MY_TIMER_PERIOD));
    }

K_TIMER_DEFINE(my_timer, my_timer_expiry_function, NULL);

int main(void)
{
    LOG_INF("Zephyr Example Application with Timer and .csv file %s\n", APP_VERSION_STRING);
    sensor = DEVICE_DT_GET(DT_NODELABEL(example_sensor));

    /* Initialize the timer and start it */
    init_my_timer();
    start_my_timer();

    while (1) {
        k_timer_status_sync(&my_timer);
    }

    return 0;
}