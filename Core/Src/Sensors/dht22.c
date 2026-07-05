#include "Sensors/dht22.h"

static void Delay_us(TIM_HandleTypeDef *htim, uint16_t us) {
    htim->Instance->CNT = 0;
    while (htim->Instance->CNT < us);
}

static bool Wait_For_Pin_State(DHT22_t *dht, GPIO_PinState state, uint16_t timeout_us) {
    dht->htim->Instance->CNT = 0;
    while (HAL_GPIO_ReadPin(dht->gpio_port, dht->gpio_pin) != state) {
        if (dht->htim->Instance->CNT > timeout_us) {
            return false; //timeout err
        }
    }
    return true;
}

void DHT22_Init(DHT22_t *dht, TIM_HandleTypeDef *htim, GPIO_TypeDef *port, uint16_t pin) {
    dht->htim = htim;
    dht->gpio_port = port;
    dht->gpio_pin = pin;
    dht->temperature = 0.0f;
    dht->humidity = 0.0f;


    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }

    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
    dht->init_ok = true;
}

bool DHT22_Read(DHT22_t *dht) {
    uint8_t data[5] = {0};
    uint16_t raw_humidity = 0;
    uint16_t raw_temperature = 0;

    uint32_t original_psc = dht->htim->Instance->PSC;
    dht->htim->Instance->PSC = 99;                 // 100MHz / (99 + 1) = 1MHz tick rate
    dht->htim->Instance->EGR = TIM_EGR_UG;         // Generate Update Event to force hardware load
    dht->htim->Instance->SR &= ~TIM_SR_UIF;        // Clear the update flag immediately

    HAL_GPIO_WritePin(dht->gpio_port, dht->gpio_pin, GPIO_PIN_RESET); // Drive Low
    Delay_us(dht->htim, 1200); // Hold low for 1.2 milliseconds

    HAL_GPIO_WritePin(dht->gpio_port, dht->gpio_pin, GPIO_PIN_SET);   // Release line (float High)
    Delay_us(dht->htim, 30);   // Wait for DHT to respond (20-40us)

    if (!Wait_For_Pin_State(dht, GPIO_PIN_RESET, 100)) goto error_exit;
    if (!Wait_For_Pin_State(dht, GPIO_PIN_SET, 100))   goto error_exit;
    if (!Wait_For_Pin_State(dht, GPIO_PIN_RESET, 100)) goto error_exit;

    for (int i = 0; i < 40; i++) {
        // Wait for the bit transmission to start (Line goes high)
        if (!Wait_For_Pin_State(dht, GPIO_PIN_SET, 100)) goto error_exit;

        // Count how long the line stays HIGH
        dht->htim->Instance->CNT = 0;
        if (!Wait_For_Pin_State(dht, GPIO_PIN_RESET, 100)) goto error_exit;
        uint16_t high_duration = dht->htim->Instance->CNT;

        data[i / 8] <<= 1;
        if (high_duration > 40) {
            data[i / 8] |= 1;
        }
    }

    dht->htim->Instance->PSC = original_psc;
    dht->htim->Instance->EGR = TIM_EGR_UG;
    dht->htim->Instance->SR &= ~TIM_SR_UIF;

    // parity checksum
    if (((data[0] + data[1] + data[2] + data[3]) & 0xFF) != data[4]) {
        return false;
    }

    //payload
    raw_humidity = (data[0] << 8) | data[1];
    raw_temperature = (data[2] << 8) | data[3];

    dht->humidity = (float)raw_humidity / 10.0f;

    if (raw_temperature & 0x8000) { // If negative sign bit is active
        dht->temperature = (float)(raw_temperature & 0x7FFF) / -10.0f;
    } else {
        dht->temperature = (float)raw_temperature / 10.0f;
    }

    return true; //data ok

error_exit:
    dht->htim->Instance->PSC = original_psc;
    dht->htim->Instance->EGR = TIM_EGR_UG;
    dht->htim->Instance->SR &= ~TIM_SR_UIF; //?
    return false;
}
