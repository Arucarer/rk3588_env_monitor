#include <stdio.h>
#include "bme280.h"


int main(void)
{
    int ret;

    bme280_data_t data;


    printf("BME280 test start\n");


    ret = bme280_init();

    if(ret < 0)
    {
        printf("BME280 init failed\n");
        return -1;
    }


    while(1)
    {

        ret = bme280_read_data(&data);

        if(ret == 0)
        {
            printf("====================\n");

            printf("Temperature: %.2f C\n",
                    data.temperature);

            printf("Humidity: %.2f %%RH\n",
                    data.humidity);

            printf("Pressure: %.2f Pa\n",
                    data.pressure);

            printf("====================\n");
        }
        else
        {
            printf("read failed\n");
        }


        sleep(1);
    }


    bme280_deinit();

    return 0;
}