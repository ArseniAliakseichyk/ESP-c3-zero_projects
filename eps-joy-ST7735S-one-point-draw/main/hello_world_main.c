#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>
#include "driver/adc.h"

#define PIN_NUM_MOSI 4
#define PIN_NUM_CLK  5
#define PIN_NUM_CS   6
#define PIN_NUM_DC   7
#define PIN_NUM_RST  8

#define X_1 0// GP0 3
#define Y_1 1// GP1 2
#define SW_1 9// GP9 10

#define X_2 3// GP0 3
#define Y_2 2// GP1 2

#define LCD_WIDTH  160
#define LCD_HEIGHT 128

spi_device_handle_t spi;

// Мини-шрифт 8x8 (только для 'H', 'E', 'L', 'O', ' ' — можно дополнить)
const uint8_t font8x8_basic[128][8] = {
    ['H'] = {
        0b10000010,
        0b10000010,
        0b10000010,
        0b11111110,
        0b10000010,
        0b10000010,
        0b10000010,
        0b00000000,
    },
    ['E'] = {
        0b11111110,
        0b10000000,
        0b10000000,
        0b11111110,
        0b10000000,
        0b10000000,
        0b11111110,
        0b00000000,
    },
    ['L'] = {
        0b10000000,
        0b10000000,
        0b10000000,
        0b10000000,
        0b10000000,
        0b10000000,
        0b11111110,
        0b00000000,
    },
    ['O'] = {
        0b01111100,
        0b10000010,
        0b10000010,
        0b10000010,
        0b10000010,
        0b10000010,
        0b01111100,
        0b00000000,
    },
    [' '] = {
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    },
    ['W'] = {
        0b10000010,
        0b10000010,
        0b10000010,
        0b10000010,
        0b10010010,
        0b10101010,
        0b11000110,
        0b00000000,
    },
    ['R'] = {
        0b11111100,
        0b10000010,
        0b10000010,
        0b11111100,
        0b10001000,
        0b10000100,
        0b10000010,
        0b00000000,
    },
    ['D'] = {
        0b11111100,
        0b10000010,
        0b10000010,
        0b10000010,
        0b10000010,
        0b10000010,
        0b11111100,
        0b00000000,
    },
    ['*'] = {
        0b00001000,
        0b00000100,
        0b11100010,
        0b00000010,
        0b11100010,
        0b00000100,
        0b00001000,
        0b00000000,
    },
    ['-'] = {
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    },
    ['0'] = {
        0b01111100,
        0b10000110,
        0b10001010,
        0b10010010,
        0b10100010,
        0b11000010,
        0b01111100,
        0b00000000,
    },
    ['1'] = {
        0b00010000,
        0b00110000,
        0b01010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b01111100,
        0b00000000,
    },
    ['2'] = {
        0b01111100,
        0b10000010,
        0b00000010,
        0b00011100,
        0b01100000,
        0b10000000,
        0b11111110,
        0b00000000,
    },
    ['3'] = {
        0b11111110,
        0b00000010,
        0b00000100,
        0b00011100,
        0b00000010,
        0b10000010,
        0b01111100,
        0b00000000,
    },
    ['4'] = {
        0b00001100,
        0b00110100,
        0b01000100,
        0b10000100,
        0b11111110,
        0b00000100,
        0b00000100,
        0b00000000,
    },
    ['5'] = {
        0b11111110,
        0b10000000,
        0b11111100,
        0b00000010,
        0b00000010,
        0b10000010,
        0b01111100,
        0b00000000,
    },
    ['6'] = {
        0b00111100,
        0b01000000,
        0b10000000,
        0b11111100,
        0b10000010,
        0b10000010,
        0b01111100,
        0b00000000,
    },
    ['7'] = {
        0b11111110,
        0b00000010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b00000000,
    },
    ['8'] = {
        0b01111100,
        0b10000010,
        0b10000010,
        0b01111100,
        0b10000010,
        0b10000010,
        0b01111100,
        0b00000000,
    },
    ['9'] = {
        0b01111100,
        0b10000010,
        0b10000010,
        0b01111110,
        0b00000010,
        0b00000100,
        0b01111000,
        0b00000000,
    },

    // Буквы
    ['X'] = {
        0b10000010,
        0b01000100,
        0b00101000,
        0b00010000,
        0b00101000,
        0b01000100,
        0b10000010,
        0b00000000,
    },
    ['Y'] = {
        0b10000010,
        0b01000100,
        0b00101000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00000000,
    },

    // Символы
    ['-'] = {
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    },
    ['+'] = {
        0b00010000,
        0b00010000,
        0b00010000,
        0b11111110,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00000000,
    },
    [':'] = {
        0b00000000,
        0b00000000,
        0b00010000,
        0b00000000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
    },
    ['S'] = {
        0b00111100,
        0b01000010,
        0b10000000,
        0b01111100,
        0b00000010,
        0b10000010,
        0b01111100,
        0b00000000,
    }

    //['-'] = {
       // 0b00000000,
       // 0b00000000,
       // 0b00000000,
       // 0b00000000,
       // 0b00000000,
       // 0b00000000,
       // 0b00000000,
       // 0b00000000,
    //}
};

// ===== LCD UTILS =====
void lcd_send_cmd(uint8_t cmd) {
    gpio_set_level(PIN_NUM_DC, 0);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd
    };
    spi_device_polling_transmit(spi, &t);
}

void lcd_send_data(uint8_t data) {
    gpio_set_level(PIN_NUM_DC, 1);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &data
    };
    spi_device_polling_transmit(spi, &t);
}

void lcd_reset() {
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
}

void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    lcd_send_cmd(0x2A);
    lcd_send_data(0x00); lcd_send_data(x0);
    lcd_send_data(0x00); lcd_send_data(x1);

    lcd_send_cmd(0x2B);
    lcd_send_data(0x00); lcd_send_data(y0);
    lcd_send_data(0x00); lcd_send_data(y1);

    lcd_send_cmd(0x2C);
}

void lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;

    lcd_set_window(x, y, x, y);
    gpio_set_level(PIN_NUM_DC, 1);

    uint8_t data[] = { color >> 8, color & 0xFF };
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = data
    };
    spi_device_polling_transmit(spi, &t);
}

void lcd_clear_screen(uint16_t color) {
    lcd_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    gpio_set_level(PIN_NUM_DC, 1);
    uint8_t line[LCD_WIDTH * 2];
    for (int i = 0; i < LCD_WIDTH; i++) {
        line[i * 2] = color >> 8;
        line[i * 2 + 1] = color & 0xFF;
    }
    for (int y = 0; y < LCD_HEIGHT; y++) {
        spi_transaction_t t = {
            .length = LCD_WIDTH * 2 * 8,
            .tx_buffer = line
        };
        spi_device_polling_transmit(spi, &t);
    }
}

// (ST7735-compatible)
void lcd_init() {
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << PIN_NUM_DC) | (1ULL << PIN_NUM_RST),
    };
    gpio_config(&io_conf);

    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_NUM_CLK,
        .max_transfer_sz = 4096,
    };
    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 1,
    };
    spi_bus_add_device(SPI2_HOST, &devcfg, &spi);

    lcd_reset();
    lcd_send_cmd(0x11); vTaskDelay(pdMS_TO_TICKS(120));
    lcd_send_cmd(0x3A); lcd_send_data(0x05); // 16-bit color
    lcd_send_cmd(0x36); lcd_send_data(0x60);
    //lcd_send_cmd(0x36); lcd_send_data(0x00); // Memory Access Control
    lcd_send_cmd(0x29); // Display ON
}

// Отображение символа 8x8
void lcd_draw_char(uint8_t ch, int x, int y, uint16_t color, uint16_t bg) {
    const uint8_t *glyph = font8x8_basic[ch];
    for (int row = 0; row < 8; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            uint16_t pixel_color = (bits & (1 << (7 - col))) ? color : bg;
            lcd_draw_pixel(x + col, y + row, pixel_color);
        }
    }
}

void lcd_draw_text(const char *text, int x, int y, uint16_t color, uint16_t bg) {
    while (*text) {
        lcd_draw_char(*text, x, y, color, bg);
        x += 8 + 1; // ширина символа + 1 пиксель отступ
        text++;
    }
}
void lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    for (uint16_t dx = 0; dx < w; dx++) {
        for (uint16_t dy = 0; dy < h; dy++) {
            if (x + dx < LCD_WIDTH && y + dy < LCD_HEIGHT) {
                lcd_draw_pixel(x + dx, y + dy, color);
            }
        }
    }
}

void lcd_draw_square(int x, int y, int size, uint16_t color) {
    size=size+4;
    int half_size = size / 2;
    // Рисуем верхнюю и нижнюю границы квадрата
    for (int i = -half_size; i < half_size; i++) {
        lcd_draw_pixel(x + i, y - half_size, color);  // Верхняя граница
        lcd_draw_pixel(x + i, y + half_size, color);  // Нижняя граница
    }

    // Рисуем левую и правую границы квадрата
    for (int j = -half_size; j < half_size; j++) {
        lcd_draw_pixel(x - half_size, y + j, color);  // Левая граница
        lcd_draw_pixel(x + half_size, y + j, color);  // Правая граница
    }
}


// Функция для рисования точки в границах квадрата
void lcd_draw_point_in_square(int x, int y, int square_x, int square_y, int square_size, uint16_t color) {
    int half_size = square_size / 2;

    lcd_draw_pixel(x, y, color);
    lcd_draw_pixel(x + 1, y, color);
    lcd_draw_pixel(x, y + 1, color);
    lcd_draw_pixel(x + 1, y + 1, color);

}


int point_x = 2200;  // Начальная координата точки по X
int point_y = 2200;  // Начальная координата точки по Y
int point2_x = 2200;  // Начальная координата точки по X
int point2_y = 2200;  // Начальная координата точки по Y


// Функция для обновления координат точки с инверсией по оси X
// Функция для обновления координат с инверсией по оси X (движение справа налево)
void update_point_position_with_inversion(int joystick_x, int joystick_y, int square_x, int square_y, int square_size, bool invert_x) {
    int max_x = 4095;  // Максимальное значение для джойстика по оси X
    int max_y = 4095;  // Максимальное значение для джойстика по оси Y

    // Масштабируем значения джойстика на экран
    point_y = square_y + ((joystick_y * square_size) / max_y) - (square_size / 2);

    // Применяем инверсию по оси X, если нужно
    if (invert_x) {
        // Инвертируем координату, но движение по оси X будет зеркальным
        point_x = square_x + ((max_x - joystick_x) * square_size) / max_x - (square_size / 2);
    } else {
        point_x = square_x + ((joystick_x * square_size) / max_x) - (square_size / 2);
    }

    // Ограничиваем точку внутри экрана
    if (point_x < 0) point_x = 0;
    if (point_x >= LCD_WIDTH) point_x = LCD_WIDTH - 1;
    if (point_y < 0) point_y = 0;
    if (point_y >= LCD_HEIGHT) point_y = LCD_HEIGHT - 1;
}
void update_point2_position_with_inversion(int joystick_x, int joystick_y, int square_x, int square_y, int square_size, bool invert_x) {
    int max_x = 4095;  // Максимальное значение для джойстика по оси X
    int max_y = 4095;  // Максимальное значение для джойстика по оси Y

    // Масштабируем значения джойстика на экран
    point2_y = square_y + ((joystick_y * square_size) / max_y) - (square_size / 2);

    // Применяем инверсию по оси X, если нужно
    if (invert_x) {
        // Инвертируем координату, но движение по оси X будет зеркальным
        point2_x = square_x + ((max_x - joystick_x) * square_size) / max_x - (square_size / 2);
    } else {
        point2_x = square_x + ((joystick_x * square_size) / max_x) - (square_size / 2);
    }

    // Ограничиваем точку внутри экрана
    if (point_x < 0) point_x = 0;
    if (point_x >= LCD_WIDTH) point_x = LCD_WIDTH - 1;
    if (point_y < 0) point_y = 0;
    if (point_y >= LCD_HEIGHT) point_y = LCD_HEIGHT - 1;
}

void no_bg_lcd_draw_char(uint8_t ch, int x, int y, uint16_t color) {
    const uint8_t *glyph = font8x8_basic[ch];
    for (int row = 0; row < 8; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << (7 - col))) {
                lcd_draw_pixel(x + col, y + row, color);  // Только пиксели текста
            }
        }
    }
}
void no_bg_lcd_draw_text(const char *text, int x, int y, uint16_t color) {
    while (*text) {
        no_bg_lcd_draw_char(*text, x, y, color);
        x += 8 + 1; // ширина символа + отступ
        text++;
    }
}

// Главная функция
void app_main() {
    lcd_init();
    lcd_clear_screen(0x0000); // чрн фон

    //lcd_draw_text("HELLO WORLD *", 10, 10, 0x07E0, 0x0000); // чёрный текст на белом фоне
    //lcd_draw_text("1234567890 +-",10,20,0x07E0, 0x0000);
    //lcd_draw_text("X Y",10,30,0x07E0, 0x0000);
    lcd_draw_text("X:", 14, 15, 0x06db, 0x0000);
    lcd_draw_text("Y:", 14, 25, 0x06db, 0x0000);
    lcd_draw_text("SW:", 5, 35, 0x06db, 0x0000);  //0x07ff cyan , 0x06db dark cyan, 0x07E0 green

    lcd_draw_text("X:", 100, 15, 0x07E0, 0x0000);
    lcd_draw_text("Y:", 100, 25, 0x07E0, 0x0000);

   
    adc1_config_width(ADC_WIDTH_BIT_12);                // 12 бит, значения от 0 до 4095
    adc1_config_channel_atten(X_1, ADC_ATTEN_DB_11);      // Полный диапазон 0–3.6V
    adc1_config_channel_atten(Y_1, ADC_ATTEN_DB_11);

    adc1_config_channel_atten(X_2, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(Y_2, ADC_ATTEN_DB_11);

    gpio_set_direction(SW_1, GPIO_MODE_INPUT);
    gpio_pullup_en(SW_1);

    char buf_x[16];
    char buf_y[16];
    char buf_sw[16];
    char old_buf_x[16];
    char old_buf_y[16];
    char old_buf_sw[16];

    char buf_x2[16], buf_y2[16];
    char old_x2[16], old_y2[16];

    int x = adc1_get_raw(X_1);
    int y = adc1_get_raw(Y_1);
    int sw = gpio_get_level(SW_1);
    //int last_x = -1, last_y = -1;
    int x2 = adc1_get_raw(X_2);
    int y2 = adc1_get_raw(Y_2);

    // Рисуем квадрат под текстом
    int square_size = 50;
    lcd_draw_square(35, 75, square_size, 0xFFFF);  // Рисуем квадрат под текстом
    lcd_draw_square(120, 75, square_size, 0xFFFF);
while (1) {
    snprintf(buf_x, sizeof(buf_x), "%d", x);
    snprintf(buf_y, sizeof(buf_y), "%d", y);
    char ch_sw=' ';
    if (sw==0)
    {
        ch_sw ='+';
    }
    else
    {
        ch_sw ='-';
    }
    snprintf(buf_sw, sizeof(buf_sw), "%c", ch_sw);
    
    snprintf(buf_x2, sizeof(buf_x2), "%d", x2);
    snprintf(buf_y2, sizeof(buf_y2), "%d", y2);

    // Очистить прямоугольник перед текстом
    //lcd_fill_rect(30, 15, 40, 8, 0x0000);  // ширина по длине текста, высота = 8
    //lcd_draw_text(old_buf_x, 30, 15, 0x0000, 0x0000);
    //lcd_draw_text(buf_x, 30, 15, 0xffff, 0x0000);
    no_bg_lcd_draw_text(old_buf_x, 30, 15, 0x0000);
    no_bg_lcd_draw_text(buf_x, 30, 15, 0xffff);


    //lcd_fill_rect(30, 25, 40, 8, 0x0000);
    //lcd_draw_text(old_buf_y, 30, 25, 0x0000, 0x0000);
    //lcd_draw_text(buf_y, 30, 25, 0xffff, 0x0000);
    no_bg_lcd_draw_text(old_buf_y, 30, 25, 0x0000);
    no_bg_lcd_draw_text(buf_y, 30, 25, 0xffff);


    //lcd_fill_rect(30, 35, 40, 8, 0x0000);
    //lcd_draw_text(old_buf_sw, 30, 35, 0x0000, 0x0000);
    //lcd_draw_text(buf_sw, 30, 35, 0xffff, 0x0000);
    no_bg_lcd_draw_text(old_buf_sw, 30, 35, 0x0000);
    no_bg_lcd_draw_text(buf_sw, 30, 35, 0xffff);


    no_bg_lcd_draw_text(old_x2, 120, 15, 0x0000);
    no_bg_lcd_draw_text(buf_x2, 120, 15, 0xffff);
    no_bg_lcd_draw_text(old_y2, 120, 25, 0x0000);
    no_bg_lcd_draw_text(buf_y2, 120, 25, 0xffff);

    strcpy(old_buf_x, buf_x);
    strcpy(old_buf_y, buf_y);
    strcpy(old_buf_sw, buf_sw);
    strcpy(old_x2, buf_x2); 
    strcpy(old_y2, buf_y2);
    
    lcd_draw_point_in_square(point_x,point_y,35,75,square_size,0x0000);
    //update_point_position(x,y,35,75,square_size);
    update_point_position_with_inversion(x,y,35,75,square_size,true);
    if (sw==0)
    {
        lcd_draw_point_in_square(point_x,point_y,35,75,square_size,0xf80c);
    }
    else
    {
        lcd_draw_point_in_square(point_x,point_y,35,75,square_size,0x06db);
    }

    // lcd_draw_point_in_square(point_x,point_y,35,75,square_size,0x06db);
    //lcd_draw_pixel(x, y, 0x07E0); // Рисуем точку внутри квадрата

    lcd_draw_point_in_square(point2_x, point2_y, 120, 75, square_size, 0x0000);
    update_point2_position_with_inversion(x2, y2, 120, 75, square_size, true);
    lcd_draw_point_in_square(point2_x, point2_y, 120, 75, square_size,0x07E0);


    x = adc1_get_raw(X_1);
    y = adc1_get_raw(Y_1);
    sw = gpio_get_level(SW_1);
    x2 = adc1_get_raw(X_2);
    y2 = adc1_get_raw(Y_2);

    //printf("X: %d Y: %d SW: %s\n", x, y, sw == 0 ? "+" : "-");

    vTaskDelay(pdMS_TO_TICKS(10));
    
}
    
}