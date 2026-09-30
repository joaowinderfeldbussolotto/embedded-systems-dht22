#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "dht.h"

#define DHT_GPIO GPIO_NUM_4
// o DHT22 usa o mesmo protocolo do AM2301 na biblioteca
#define DHT_TIPO DHT_TYPE_AM2301

static const char *TAG = "dht22";

static void tarefa_dht(void *arg)
{
    float umidade, temperatura;

    while (1) {
        esp_err_t err = dht_read_float_data(DHT_TIPO, DHT_GPIO, &umidade, &temperatura);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Temperatura: %.1f C | Umidade: %.1f %%", temperatura, umidade);
        } else {
            ESP_LOGE(TAG, "Falha na leitura: %s", esp_err_to_name(err));
        }
        // o sensor precisa de pelo menos 2 s entre leituras
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{
    // a biblioteca usa o pino em dreno aberto, entao precisa do pull-up pra subir o nivel
    gpio_set_pull_mode(DHT_GPIO, GPIO_PULLUP_ONLY);

    ESP_LOGI(TAG, "Iniciando leitura do DHT22 no GPIO%d", DHT_GPIO);
    xTaskCreate(tarefa_dht, "tarefa_dht", 4096, NULL, 5, NULL);
}
