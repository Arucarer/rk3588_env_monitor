#include "mock_sensor.h"

#include <stddef.h>//NULL
#include <string.h>
#include <time.h>

int mock_sensor_init(void)
{
    return 0;
}

int mock_sensor_collect(sensor_data_t *data)
{
    if (data == NULL) {
        return -1;
    }

    memset(data, 0, sizeof(sensor_data_t));

    data->bme_temperature = 25.0f;
    data->bme_humidity = 55.0f;
    data->pressure = 1013.25f;

    data->temperature = 24.8f;
    data->humidity = 54.5f;

    data->soil_humidity = 45.0f;

    data->rain_detected = false;
    data->rainfall = 0.0f;

    data->valid = true;
    data->timestamp = (uint64_t)time(NULL);

    return 0;
}

void mock_sensor_deinit(void)
{
}