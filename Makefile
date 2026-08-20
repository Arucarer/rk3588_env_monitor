CC := /home/aaa/rk3588_linux_sdk/buildroot/output/rockchip_atk_dlrk3588/host/bin/aarch64-buildroot-linux-gnu-gcc

TARGET := env_monitor

CFLAGS := -Wall -Wextra -std=c11
INCLUDES := -Iinclude -Isensor -Idriver -Imodbus -Idatabase -Imqtt
LDLIBS := -lsqlite3 -lpaho-mqtt3c

SOURCES := \
	app/main.c \
	sensor/sensor_manager.c \
	sensor/mock_sensor.c \
	sensor/bme280.c \
	sensor/sht30_rs485.c \
	sensor/soil.c \
	sensor/rain.c \
	driver/i2c.c \
	driver/uart.c \
	driver/adc.c \
	driver/gpio.c \
	modbus/modbus_master.c \
	modbus/crc16.c \
	database/sqlite_manager.c \
	mqtt/mqtt_client.c

OBJECTS := $(SOURCES:.c=.o)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean