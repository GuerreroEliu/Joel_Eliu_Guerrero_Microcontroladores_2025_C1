#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"

#define LED_GPIO GPIO_NUM_4
#define PWM_GPIO GPIO_NUM_2
#define MONO_LED_GPIO GPIO_NUM_5
#define MONO_BTN_GPIO GPIO_NUM_18

static const char *TAG = "ESP32_WebControl";

// Astable
static bool blink_enabled = false;
static double blink_period_ms = 1000.0;

// PWM
static int pwm_duty = 512;
static const int pwm_freq = 1000;

// Monoestable
static int64_t btn_press_time = 0;
static char mono_msg[64] = "Aún no se ha presionado el botón.";

// PWM inicial
void setup_pwm(int duty) {
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = pwm_freq,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t channel = {
        .gpio_num = PWM_GPIO,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = duty,
        .hpoint = 0
    };
    ledc_channel_config(&channel);
}

// Tarea astable
void blink_task(void *pvParameter) {
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    while (1) {
        if (blink_enabled) {
            gpio_set_level(LED_GPIO, 1);
            vTaskDelay(pdMS_TO_TICKS(blink_period_ms / 2));
            gpio_set_level(LED_GPIO, 0);
            vTaskDelay(pdMS_TO_TICKS(blink_period_ms / 2));
        } else {
            gpio_set_level(LED_GPIO, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

// ISR del botón
static void IRAM_ATTR button_isr_handler(void* arg) {
    int level = gpio_get_level(MONO_BTN_GPIO);
    int64_t now = esp_timer_get_time();  // en microsegundos

    if (level == 1) {
        btn_press_time = now;
    } else {
        int64_t duration = (now - btn_press_time) / 1000000;  // seg
        btn_press_time = 0;

        // Encender LED por la duración
        gpio_set_level(MONO_LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(duration * 1000));
        gpio_set_level(MONO_LED_GPIO, 0);

        // Guardar mensaje
        snprintf(mono_msg, sizeof(mono_msg),
                 "El botón fue presionado por %lld segundos", duration);
    }
}

// Página web
esp_err_t root_get_handler(httpd_req_t *req) {
    char html[2048];
    snprintf(html, sizeof(html),
        "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<title>ESP32 Control</title><style>"
        "body { font-family:sans-serif; padding:2rem; text-align:center; background:#f4f4f4; }"
        "input, button { padding:10px; margin:10px; width:80%%; max-width:300px; font-size:16px; }"
        "button { background:#28a745; color:white; border:none; border-radius:5px; }"
        "</style></head><body>"
        "<h2>🌀 Astable - GPIO 4</h2>"
        "<input type='number' id='resistencia' placeholder='Resistencia (Ω)' step='any'><br>"
        "<input type='number' id='capacitancia' placeholder='Capacitancia (μF)' step='any'><br>"
        "<button onclick='calcularFrecuencia()'>Aplicar</button>"
        "<p id='resultado'></p><hr>"
        "<h2>🔧 PWM - GPIO 2</h2>"
        "<input type='range' min='0' max='100' value='50' id='pwmSlider'><br>"
        "<p>Duty Cycle: <span id='dutyValue'>50</span>%%</p>"
        "<button onclick='aplicarPWM()'>Ejecutar</button><hr>"
        "<h2>🟢 Monoestable - GPIO 5 (LED) / GPIO 18 (Botón)</h2>"
        "<p>%s</p>"
        "<script>"
        "function calcularFrecuencia() {"
        "let R=parseFloat(document.getElementById('resistencia').value);"
        "let C=parseFloat(document.getElementById('capacitancia').value);"
        "if(R<=0||C<=0||isNaN(R)||isNaN(C)){document.getElementById('resultado').innerText='Ingrese valores válidos';return;}"
        "let T=2*R*(C/1000000);let f=1/T;let ms=T*1000;"
        "document.getElementById('resultado').innerText='Frecuencia: '+f.toFixed(2)+' Hz';"
        "fetch('/set?periodo='+ms.toFixed(2));}"
        "document.getElementById('pwmSlider').oninput=function(){"
        "document.getElementById('dutyValue').innerText=this.value;};"
        "function aplicarPWM(){let d=document.getElementById('pwmSlider').value;"
        "fetch('/pwm?duty='+d);}"
        "</script></body></html>", mono_msg);

    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// /set para astable
esp_err_t set_get_handler(httpd_req_t *req) {
    char buf[64];
    char param[16];
    if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
        if (httpd_query_key_value(buf, "periodo", param, sizeof(param)) == ESP_OK) {
            blink_period_ms = atof(param);
            blink_enabled = true;
        }
    }
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

// /pwm para duty cycle
esp_err_t pwm_get_handler(httpd_req_t *req) {
    char buf[64];
    char param[8];
    if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
        if (httpd_query_key_value(buf, "duty", param, sizeof(param)) == ESP_OK) {
            int percent = atoi(param);
            pwm_duty = (1023 * percent) / 100;
            setup_pwm(pwm_duty);
        }
    }
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

// Servidor web
httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root = { .uri = "/", .method = HTTP_GET, .handler = root_get_handler };
        httpd_uri_t set = { .uri = "/set", .method = HTTP_GET, .handler = set_get_handler };
        httpd_uri_t pwm = { .uri = "/pwm", .method = HTTP_GET, .handler = pwm_get_handler };
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &set);
        httpd_register_uri_handler(server, &pwm);
    }
    return server;
}

// Init Wi-Fi
void wifi_init_softap() {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "ESP32_Control",
            .ssid_len = strlen("ESP32_Control"),
            .channel = 1,
            .authmode = WIFI_AUTH_OPEN,
            .max_connection = 4
        }
    };
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();
    ESP_LOGI(TAG, "WiFi AP Iniciado: ESP32_Control");
}

void app_main(void) {
    nvs_flash_init();
    wifi_init_softap();
    start_webserver();
    setup_pwm(pwm_duty);
    gpio_reset_pin(MONO_LED_GPIO);
    gpio_set_direction(MONO_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(MONO_BTN_GPIO);
    gpio_set_direc tion(MONO_BTN_GPIO, GPIO_MODE_INPUT);
    gpio_set_intr_type(MONO_BTN_GPIO, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(MONO_BTN_GPIO, button_isr_handler, NULL);
    xTaskCreate(blink_task, "blink_task", 2048, NULL, 5, NULL);
}