#include "dht22.h"

static void DHT_PinOut(DHT22_HandleTypeDef *d)
{
    GPIO_InitTypeDef g = {0};
    g.Pin = d->GPIO_Pin;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(d->GPIOx, &g);
}

static void DHT_PinIn(DHT22_HandleTypeDef *d)
{
    GPIO_InitTypeDef g = {0};
    g.Pin = d->GPIO_Pin;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(d->GPIOx, &g);
}

static void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

uint8_t DHT22_Read(DHT22_HandleTypeDef *d, float *temp, float *hum)
{
    uint8_t data[5] = {0};

    // START SIGNAL
    DHT_PinOut(d);
    HAL_GPIO_WritePin(d->GPIOx, d->GPIO_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);                 // 1 ms
    HAL_GPIO_WritePin(d->GPIOx, d->GPIO_Pin, GPIO_PIN_SET);
    delay_us(30);
    DHT_PinIn(d);

    // WAIT FOR RESPONSE (tuned for 16 MHz)

    while(HAL_GPIO_ReadPin(d->GPIOx, d->GPIO_Pin) != GPIO_PIN_RESET)
    {
		lcd_print("Waiting for LOW", 1);
    }

    while(HAL_GPIO_ReadPin(d->GPIOx, d->GPIO_Pin) != GPIO_PIN_SET)
    {
		lcd_print("Waiting for HIGH", 1);
    }

    // READ 40 BITS
    for (int i = 0; i < 40; i++)
    {
        // wait for HIGH
        while (HAL_GPIO_ReadPin(d->GPIOx, d->GPIO_Pin) == GPIO_PIN_RESET);

        uint32_t start = DWT->CYCCNT;

        // wait for LOW
        while (HAL_GPIO_ReadPin(d->GPIOx, d->GPIO_Pin) == GPIO_PIN_SET);

        uint32_t width = DWT->CYCCNT - start;

        // threshold tuned for 16 MHz
        uint8_t bit = (width > (SystemCoreClock / 2500)) ? 1 : 0;

        data[i / 8] <<= 1;
        data[i / 8] |= bit;
    }

    // NO CHECKSUM — DIRECT CONVERSION
    uint16_t rh = (data[0] << 8) | data[1];
    uint16_t t  = (data[2] << 8) | data[3];

    *hum = rh / 10.0f;

    if (t & 0x8000) {
        t &= 0x7FFF;
        *temp = -(t / 10.0f);
    } else {
        *temp = t / 10.0f;
    }

    return 0;
}
