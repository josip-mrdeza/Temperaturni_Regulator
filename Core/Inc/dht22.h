#ifndef DHT22_H
#define DHT22_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef struct {
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
    uint8_t flag_it;
} DHT22_HandleTypeDef;

void DHT22_Init(DHT22_HandleTypeDef *dht);
uint8_t DHT22_Read(DHT22_HandleTypeDef *dht, float *temperature, float *humidity);

#endif
