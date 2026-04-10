#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_log.h"
 
#include "wifi_utile.h"
#include "mqtt_utile.h"

#define STEP	16
#define DIR		15
#define ENA		7
#define STEP2	6
#define DIR2	5
#define ENA2	4

#define SORT 	9
#define RENTRE	11
#define ENALIN	10

#define LEDC_FREQ_MOT 20000 // Hz

#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_FREQ 2666 // Hz
#define LEDC_RES LEDC_TIMER_8_BIT // 0-255

// Timer 0 → Canal 0-1
#define TIMER_A LEDC_TIMER_0
#define CHANNEL_A LEDC_CHANNEL_0
#define CHANNEL_B LEDC_CHANNEL_1

//Timer 1
#define TIMER_B LEDC_TIMER_1
#define CHANNEL_C LEDC_CHANNEL_2
#define CHANNEL_D LEDC_CHANNEL_3

static const char *TAG = "main";
 
/* ── Callback appelé quand l'ESP32 reçoit un message sur nate/commande ───── */
static void on_mqtt_message(const char *topic, int topic_len,
                             const char *data,  int data_len)
{
    ESP_LOGI(TAG, "Message reçu!");
    ESP_LOGI(TAG, "  Topic : %.*s", topic_len, topic);
    ESP_LOGI(TAG, "  Data  : %.*s", data_len,  data);
 
    /* Exemple : réagir à une commande */
    if (strncmp(data, "D", data_len) == 0) {
        ESP_LOGI(TAG, "Commande D reçue");
		gpio_set_level(DIR, 1);
		gpio_set_level(DIR2, 0);
    } else if (strncmp(data, "M", data_len) == 0) {
        ESP_LOGI(TAG, "Commande M reçue");
		gpio_set_level(DIR, 0);
		gpio_set_level(DIR2, 1);
    } else if (strncmp(data, "1", data_len) == 0) {
        ESP_LOGI(TAG, "Commande 1 reçue");
		gpio_set_level(ENA, 1);
		gpio_set_level(ENA2, 1);
		gpio_set_level(ENALIN, 1);
	} else if (strncmp(data, "0", data_len) == 0) {
	    ESP_LOGI(TAG, "Commande 0 reçue");
		gpio_set_level(ENA, 0);
		gpio_set_level(ENA2, 0);
		gpio_set_level(ENALIN, 0);
	} else 	if (strncmp(data, "sort", data_len) == 0) {
	    ESP_LOGI(TAG, "Commande sort reçue");
		ledc_set_duty(LEDC_MODE, CHANNEL_C, 128);
		ledc_update_duty(LEDC_MODE, CHANNEL_C);
		ledc_set_duty(LEDC_MODE, CHANNEL_D, 0);
		ledc_update_duty(LEDC_MODE, CHANNEL_D);
	}  else if (strncmp(data, "rentre", data_len) == 0) {
	    ESP_LOGI(TAG, "Commande rentre reçue");
		ledc_set_duty(LEDC_MODE, CHANNEL_C, 0);
		ledc_update_duty(LEDC_MODE, CHANNEL_C);
		ledc_set_duty(LEDC_MODE, CHANNEL_D, 128);
		ledc_update_duty(LEDC_MODE, CHANNEL_D);
		}
	}
	

void app_main(void)
{
	// --- Timer B ---
			ledc_timer_config_t timer_b = {
				.speed_mode = LEDC_MODE,
				.timer_num = TIMER_B,
				.duty_resolution = LEDC_RES,
				.freq_hz = LEDC_FREQ_MOT,
				.clk_cfg = LEDC_AUTO_CLK
			};
			ledc_timer_config(&timer_b);
			
			ledc_channel_config_t channel_c = {
				.gpio_num = SORT,
				.speed_mode = LEDC_MODE,
				.channel = CHANNEL_C,
				.intr_type = LEDC_INTR_DISABLE,
				.timer_sel = TIMER_B,
				.duty = 0,
				.hpoint = 0
			};
			ledc_channel_config(&channel_c);

			ledc_channel_config_t channel_d = {
				.gpio_num = RENTRE,
				.speed_mode = LEDC_MODE,
				.channel = CHANNEL_D,
				.intr_type = LEDC_INTR_DISABLE,
				.timer_sel = TIMER_B,
				.duty = 0,
				.hpoint = 0
			};
			ledc_channel_config(&channel_d);
			
	// --- Timer A ---
		ledc_timer_config_t timer_a = {
			.speed_mode = LEDC_MODE,
			.timer_num = TIMER_A,
			.duty_resolution = LEDC_RES,
			.freq_hz = LEDC_FREQ,
			.clk_cfg = LEDC_AUTO_CLK
		};
		ledc_timer_config(&timer_a);
		
		ledc_channel_config_t channel_a = {
			.gpio_num = STEP,
			.speed_mode = LEDC_MODE,
			.channel = CHANNEL_A,
			.intr_type = LEDC_INTR_DISABLE,
			.timer_sel = TIMER_A,
			.duty = 0,
			.hpoint = 0
		};
		ledc_channel_config(&channel_a);

		ledc_channel_config_t channel_b = {
			.gpio_num = STEP2,
			.speed_mode = LEDC_MODE,
			.channel = CHANNEL_B,
			.intr_type = LEDC_INTR_DISABLE,
			.timer_sel = TIMER_A,
			.duty = 0,
			.hpoint = 0
		};
		ledc_channel_config(&channel_b);

		
		//gpio_reset_pin(STEP);
		gpio_reset_pin(DIR);
		gpio_reset_pin(ENA);
		gpio_reset_pin(DIR2);
		gpio_reset_pin(ENA2);
		gpio_reset_pin(ENALIN);
		
		//gpio_set_direction(STEP, GPIO_MODE_OUTPUT);
		gpio_set_direction(DIR, GPIO_MODE_OUTPUT);
		gpio_set_direction(ENA, GPIO_MODE_OUTPUT);
		gpio_set_direction(ENA2, GPIO_MODE_OUTPUT);
		gpio_set_direction(DIR2, GPIO_MODE_OUTPUT);
		gpio_set_direction(ENALIN, GPIO_MODE_OUTPUT);
		
		gpio_set_level(ENA, 0);
		gpio_set_level(DIR, 1);
		gpio_set_level(ENA2, 0);
		gpio_set_level(DIR2, 0);
		gpio_set_level(ENALIN, 0);
		
		ledc_set_duty(LEDC_MODE, CHANNEL_A, 128);
		ledc_update_duty(LEDC_MODE, CHANNEL_A);
		
		ledc_set_duty(LEDC_MODE, CHANNEL_B, 128);
		ledc_update_duty(LEDC_MODE, CHANNEL_B);
		
		/*ledc_set_duty(LEDC_MODE, CHANNEL_C, 128);
		ledc_update_duty(LEDC_MODE, CHANNEL_C);*/

		/*ledc_set_duty(LEDC_MODE, CHANNEL_D, 128);
		ledc_update_duty(LEDC_MODE, CHANNEL_D);*/
    /* 1. NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
 
    /* 2. WiFi — bloque jusqu'à IP obtenue */
    if (!wifi_manager_init()) {
        ESP_LOGE(TAG, "WiFi failed, arrêt.");
        return;
    }
 
    /* 3. MQTT — démarre et subscribe automatiquement */
    mqtt_manager_init(on_mqtt_message);
 
    int count = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        if (mqtt_manager_is_connected()) {
            char msg[32];
            snprintf(msg, sizeof(msg), "ping %d", count++);
            mqtt_manager_publish(msg, 0, 0);
        }
    }
}
