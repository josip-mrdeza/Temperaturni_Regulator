/*
 * dht22.c
 *
 *  Created on: Jul 5, 2026
 *      Author: joki
 */


#ifndef DHT22_H
#define DHT22_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

typedef struct {
    TIM_HandleTypeDef *htim;    // Ptr to TIM1 handle
    GPIO_TypeDef *gpio_port;    // GPIOB
    uint16_t gpio_pin;          // GPIO_PIN_0
    float temperature;
    float humidity;
    _Bool init_ok;
} DHT22_t;

// Public Methods
void DHT22_Init(DHT22_t *dht, TIM_HandleTypeDef *htim, GPIO_TypeDef *port, uint16_t pin);
bool DHT22_Read(DHT22_t *dht);

#endif // DHT22_H
