#include <stdio.h> 
#include "esp_log.h" // บันทึก Log
#include "driver/i2c.h" // โมดูล I2C 
#include "i2c_lcd.h" // ควบคุมจอ LCD ผ่าน I2C
#include <stdbool.h> // boolean (true/false)
#include <freertos/FreeRTOS.h> // Real-Time Operating
#include <freertos/task.h> // Task & Scheduling
#include <ultrasonic.h> // Ultrasonic sensor
#include <esp_err.h>  // Error
#include "driver/gpio.h" // ขา GPIO

/* ขา ultrasonic */
#define MAX_DISTANCE_CM 500 // 5m max 
#define TRIGGER_GPIO 5 
#define ECHO_GPIO 18 
/* ขา LED */
#define RED_LED_GPIO 26 // Define GPIO for red_led
#define BLUE_LED_GPIO 25 // Define GPIO for blue_led
/* ขา buzzer */
#define BUZZER_GPIO 23 // Define GPIO for buzzer


static const char *TAG = "i2c-simple-example"; // TAG บันทึก Log ของโปรแกรม


float distance;

/* ช่วยให้ผู้เข้าใจโค้ดสามารถเข้าใจหรือเข้าถึงการทำงานของโค้ดได้ง่ายขึ้น
โดยไม่ต้องดูรายละเอียดในโค้ดต้นฉบับทุกครั้งที่ต้องการ*/
/**
 * @brief i2c master initialization
 */

/* ฟังก์ชัน เริ่มต้นการทำงานของโมดูล I2C เป็นโหมด Master */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_NUM_0;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000, //100000kHZ
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}

void ultrasonic_test(void *pvParameters)
{
    ultrasonic_sensor_t sensor = {
        .trigger_pin = TRIGGER_GPIO,
        .echo_pin = ECHO_GPIO
    };

    ultrasonic_init(&sensor);

    while (true)
    {
        esp_err_t res = ultrasonic_measure(&sensor, MAX_DISTANCE_CM, &distance);
        if (res != ESP_OK)
        {
            printf("Error%d:", res);
            switch (res)
            {
                case ESP_ERR_ULTRASONIC_PING:
                    printf("Cannot ping (device is in invalid state)\n");
                    break;
                case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
                    printf("Ping timeout (no device found)\n");
                    break;
                case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
                    printf("Echo timeout (i.e. distance too big)\n");
                    break;
                default:
                    printf("%s\n", esp_err_to_name(res));
            }
        }
        else
        {
            char buffer[20];
            sprintf(buffer, "Distance:%0.04fm", distance);
            lcd_put_cur(0, 0);
            lcd_send_string(buffer);
            vTaskDelay(pdMS_TO_TICKS(500));

            gpio_set_direction(BLUE_LED_GPIO, GPIO_MODE_OUTPUT);
            gpio_set_direction(RED_LED_GPIO, GPIO_MODE_OUTPUT);
            
            gpio_set_direction(BUZZER_GPIO, GPIO_MODE_OUTPUT);

            if (distance >= 0.1500) {
                gpio_set_level(BLUE_LED_GPIO, 1); // Turn on blue LED
                gpio_set_level(RED_LED_GPIO, 0); // Turn off red LED
                gpio_set_level(BUZZER_GPIO, 0); // Turn off buzzer
                // vTaskDelay(2000 / portTICK_PERIOD_MS); // wait for 2 second

            } else {
                gpio_set_level(RED_LED_GPIO, 1); // Turn on red LED
                gpio_set_level(BLUE_LED_GPIO, 0); // Turn off blue LED
                gpio_set_level(BUZZER_GPIO, 1);// Turn on buzzer
                // vTaskDelay(2000 / portTICK_PERIOD_MS); // wait for 2 second
            }
        }
    }

}

void app_main(void)
{
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    lcd_init();
    lcd_clear();

    xTaskCreate(ultrasonic_test, "ultrasonic_test", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
}
