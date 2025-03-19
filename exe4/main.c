#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

QueueHandle_t xQueueButIdR;
SemaphoreHandle_t xSemaphore_r;

QueueHandle_t xQueueButIdG;
SemaphoreHandle_t xSemaphore_g;

void btn_callback_r(uint gpio, uint32_t events) {
    if (events == 0x4) { // falling edge
        xSemaphoreGiveFromISR(xSemaphore_r, 0);
    }
}

void btn_callback_g(uint gpio, uint32_t events) {
    if (events == 0x4) { // falling edge
        xSemaphoreGiveFromISR(xSemaphore_g, 0);
    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueButIdR, &delay, 0)) {
            printf("LED R delay: %d\n", delay);
        }

        if (delay > 0) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

void btn_1_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback_r);

    int delay = 0;
    while (true) {
        if (xSemaphoreTake(xSemaphore_r, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (delay < 1000) {
                delay += 100;
            } else {
                delay = 100;
            }
            printf("BTN R delay: %d\n", delay);
            xQueueSend(xQueueButIdR, &delay, 0);
        }
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueButIdG, &delay, 0)) {
            printf("LED G delay: %d\n", delay);
        }

        if (delay > 0) {
            gpio_put(LED_PIN_G, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_G, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

void btn_2_task(void *p) {
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);
    gpio_set_irq_enabled_with_callback(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true, &btn_callback_g);

    int delay = 0;
    while (true) {
        if (xSemaphoreTake(xSemaphore_g, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (delay < 1000) {
                delay += 100;
            } else {
                delay = 100;
            }
            printf("BTN G delay: %d\n", delay);
            xQueueSend(xQueueButIdG, &delay, 0);
        }
    }
}

int main() {
    stdio_init_all();
    printf("Start RTOS \n");

    xQueueButIdR = xQueueCreate(32, sizeof(int));
    xSemaphore_r = xSemaphoreCreateBinary();

    xQueueButIdG = xQueueCreate(32, sizeof(int));
    xSemaphore_g = xSemaphoreCreateBinary();

    xTaskCreate(led_1_task, "LED_Task_R", 256, NULL, 1, NULL);
    xTaskCreate(btn_1_task, "BTN_Task_R", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task_G", 256, NULL, 1, NULL);
    xTaskCreate(btn_2_task, "BTN_Task_G", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
