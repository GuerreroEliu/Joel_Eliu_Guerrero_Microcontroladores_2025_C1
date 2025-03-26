#include <stdio.h>
#include "driver/gpio.h"
#include "driver/timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#define LED_GPIO GPIO_NUM_5       
#define PULSADOR_GPIO GPIO_NUM_4  
#define TIEMPO_ANTIREBOTE 30      

static TimerHandle_t temporizador_parpadeo;
static QueueHandle_t cola_pulsaciones;
static volatile uint32_t duracion_pulsacion = 0;
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR manejar_interrupcion(void *arg) {
    static uint32_t momento_inicio = 0;
    static uint32_t ultima_interrupcion = 0;
    uint32_t ahora = xTaskGetTickCountFromISR();
    
    
    if ((ahora - ultima_interrupcion) < pdMS_TO_TICKS(TIEMPO_ANTIREBOTE)) {
        return;
    }
    ultima_interrupcion = ahora;
    
    if (gpio_get_level(PULSADOR_GPIO)) {
        // Botón liberado
        portENTER_CRITICAL_ISR(&mux);
        duracion_pulsacion = (esp_timer_get_time() - momento_inicio) / 1000;
        portEXIT_CRITICAL_ISR(&mux);
        xQueueSendFromISR(cola_pulsaciones, &duracion_pulsacion, NULL);
    } else {
        // Botón presionado
        momento_inicio = esp_timer_get_time();
        gpio_set_level(LED_GPIO, 1);
    }
}

void controlar_parpadeo(TimerHandle_t xTemporizador) {
    static bool estado_led = false;
    estado_led = !estado_led;
    gpio_set_level(LED_GPIO, estado_led);
}

void tarea_procesar_pulsacion(void *arg) {
    uint32_t duracion;
    while (1) {
        if (xQueueReceive(cola_pulsaciones, &duracion, portMAX_DELAY)) {
            printf("Pulsación detectada: %d ms\n", duracion);
            
            gpio_set_level(LED_GPIO, 0);
            xTimerChangePeriod(temporizador_parpadeo, pdMS_TO_TICKS(duracion / 2), 0);
            xTimerStart(temporizador_parpadeo, 0);
            
            vTaskDelay(pdMS_TO_TICKS(duracion * 3)); 
            xTimerStop(temporizador_parpadeo, 0);
            gpio_set_level(LED_GPIO, 0);
        }
    }
}

void iniciar_aplicacion() {
    // Configuración de hardware
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    
    gpio_reset_pin(PULSADOR_GPIO);
    gpio_set_direction(PULSADOR_GPIO, GPIO_MODE_INPUT);
    gpio_pullup_en(PULSADOR_GPIO);
    gpio_set_intr_type(PULSADOR_GPIO, GPIO_INTR_ANYEDGE);
    
    
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    gpio_isr_handler_add(PULSADOR_GPIO, manejar_interrupcion, NULL);
    
    
    cola_pulsaciones = xQueueCreate(5, sizeof(uint32_t));
    temporizador_parpadeo = xTimerCreate("Parpadeo", pdMS_TO_TICKS(500), pdTRUE, NULL, controlar_parpadeo);
    
    
    xTaskCreate(tarea_procesar_pulsacion, "Procesar Pulsacion", 2560, NULL, 12, NULL);
    
    printf("Sistema listo. Presione el pulsador para comenzar.\n");
}

void app_main() {
    iniciar_aplicacion();
}