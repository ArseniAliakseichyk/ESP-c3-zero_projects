#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include <stdio.h>

#define PIN_MOSI 7
#define PIN_CLK 10
#define PIN_CS 8

#define X ADC1_CHANNEL_1 // GP4
#define Y ADC1_CHANNEL_0 // GP5
#define SW GPIO_NUM_6     // GP6

spi_device_handle_t spi;

void max7219_write(uint8_t reg, uint8_t data) {
    spi_transaction_t t = {
        .length = 16,
        .flags = SPI_TRANS_USE_TXDATA,
        .tx_data = {reg, data}
    };
    spi_device_polling_transmit(spi, &t);
}

void init_display() {
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    };
    spi_device_interface_config_t devcfg = {
        .mode = 0,
        .clock_speed_hz = 10000000,
        .spics_io_num = PIN_CS,
        .queue_size = 1
    };

    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_DISABLED);
    spi_bus_add_device(SPI2_HOST, &devcfg, &spi);

    max7219_write(0x0C, 0x01);  // Выход из shutdown
    max7219_write(0x09, 0x00);  // Без декодирования цифр
    max7219_write(0x0B, 0x07);  // Все 8 строк
    max7219_write(0x0A, 0x08);  // Яркость средняя
    max7219_write(0x0F, 0x00);  // Отключить тестовый режим
}

void clear_display() {
    for (int row = 1; row <= 8; row++) {
        max7219_write(row, 0x00);  // Очистить все строки
    }
}

void set_pixel(int x, int y) {
    clear_display();
    max7219_write(y + 1, 1 << (7 - x));  // Установить точку в нужной позиции
}

void app_main() {
    init_display();

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(X, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(Y, ADC_ATTEN_DB_11);

    gpio_set_direction(SW, GPIO_MODE_INPUT);
    gpio_pullup_en(SW);

    int last_x = -1, last_y = -1;

    while (1) {
        int x = (adc1_get_raw(X) * 8) / 4096;
        int y = (adc1_get_raw(Y) * 8) / 4096;

        if (x != last_x || y != last_y) {
            set_pixel(x, y);
            last_x = x;
            last_y = y;
        }

        vTaskDelay(pdMS_TO_TICKS(70));
    }
}
