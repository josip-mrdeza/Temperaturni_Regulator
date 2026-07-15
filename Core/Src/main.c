/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LCD/ssd1315.h"
#include "Sensors/dht22.h"
#include <stdio.h>
#include <stdbool.h>
#include "usbd_cdc_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
AppData_t guiData;
uint32_t last_tick;
DHT22_t dht_instance; //class ref
volatile bool trigger_adc_read;
volatile bool trigger_dht_read;
volatile uint8_t usb_rx_buff[128];
volatile uint16_t usb_rx_buff_len;
volatile uint8_t usb_rx_buff_flag = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Usb_Rx_Draw(volatile uint8_t usb_rx_buff_flag,
		volatile uint16_t usb_rx_buff_len, AppData_t *guiData) {
	if (usb_rx_buff_flag && usb_rx_buff_len > 0) {
		usb_rx_buff_flag = 0;
		guiData->currentState = STATE_USB_RX;
		Menu_Draw(&hi2c2, &*guiData);
		HAL_Delay(1000);
	}
}
void Dht_Read_Draw(volatile _Bool trigger_dht_read,
		DHT22_t *dht_instance, AppData_t *guiData) {
	/* USER CODE END WHILE */
	/* USER CODE BEGIN 3 */
	if (trigger_dht_read && DHT22_Read(dht_instance)) {
		trigger_dht_read = 0;
		guiData->currentState = STATE_TEMP_ADJUST;
		guiData->currentTemperature = dht_instance->temperature;
		guiData->currentHumidity = dht_instance->humidity;
		Menu_Draw(&hi2c2, guiData);
		uint8_t usb_buf_temp[128];
		int len = sprintf((char*restrict) usb_buf_temp,
				"Temp: %.1f C | Hum: %.1f %%\r\n",
				dht_instance->temperature, dht_instance->humidity);
		CDC_Transmit_FS((uint8_t*) usb_buf_temp, len);
	}
}

void ADC_Read_Draw(volatile _Bool trigger_adc_read) {
	if (trigger_adc_read) {
		//12 bit adc
		trigger_adc_read = 0;
		HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY); //IN0
		uint32_t in0_val = HAL_ADC_GetValue(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY); //IN
		uint32_t in1_val = HAL_ADC_GetValue(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY); //silicon_temp
		uint32_t si_board_temp_val = HAL_ADC_GetValue(&hadc1);

		uint32_t adc_max = (1<<12) - 1;
		float in0_voltage = (in0_val / adc_max) * 3.3f;
		float shunt_in1_voltage = (in1_val / adc_max) * 3.3f;
		float si_temp_voltage = (si_board_temp_val / adc_max) * 3.3f;

		float shunt_resistance = 1; //triba se izmirit multimetron tocno
		float shunt_current = shunt_in1_voltage / shunt_resistance;
		float si_temp = ((si_temp_voltage - 0.76f) / 0.0025f) + 25.0f; //formula sa stm32 stranice

		guiData.system_voltage = in0_voltage;
		guiData.system_current = shunt_current;
		guiData.silicon_temp = si_temp;
	}
}

/* USER CODE END 0 */

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */

int main(void)
{

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_ADC1_Init();
	MX_TIM2_Init();
	MX_I2C2_Init();
	MX_TIM3_Init();
	MX_USB_DEVICE_Init();
	/* USER CODE BEGIN 2 */
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	init_all();
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		Dht_Read_Draw(trigger_dht_read, &dht_instance, &guiData);
		Usb_Rx_Draw(usb_rx_buff_flag, usb_rx_buff_len, &guiData);
		ADC_Read_Draw(trigger_adc_read);
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 192;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void Servo_Init(void) {
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

void Servo_SetAngle(uint8_t angle) {
	if (angle > 180) angle = 180;

	float pulse_width = (((1830.0f) / 180.0f) * angle) + 500.0f; //nastelano da bude accurate

	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse_width);
}
void init_all()
{
	Menu_Init(&guiData);
	SSD1315_Init(&hi2c2);
	Menu_Draw(&hi2c2, &guiData);
	char buff[24];
	sprintf(buff, "DHT22 : ...");
	SSD1315_DrawString(10, 20, buff);
	SSD1315_UpdateScreen(&hi2c2);
	DHT22_Init(&dht_instance, &htim3, GPIOB, GPIO_PIN_0);
	sprintf(buff, "DHT22 : %s", dht_instance.init_ok ? "Ok" : "Failed");
	SSD1315_DrawString(10, 20, buff);
	SSD1315_UpdateScreen(&hi2c2);
	sprintf(buff, "Timers : ...");
	SSD1315_DrawString(10, 36, buff);
	SSD1315_UpdateScreen(&hi2c2);
	Servo_Init();
	HAL_TIM_Base_Start_IT(&htim3);
	sprintf(buff, "Timers : Ok");
	SSD1315_DrawString(10, 36, buff);
	SSD1315_UpdateScreen(&hi2c2);
	sprintf(buff, "ADC : ...");
	SSD1315_DrawString(10, 36, buff);
	SSD1315_UpdateScreen(&hi2c2);
	sprintf(buff, "ADC : %s", HAL_ADC_Start(&hadc1) == HAL_OK ? "Ok" : "Failed");
	SSD1315_DrawString(10, 36, buff);
	SSD1315_UpdateScreen(&hi2c2);
	sprintf(buff, "USB-OTG : ...");
	SSD1315_DrawString(10, 20, buff);
	SSD1315_UpdateScreen(&hi2c2);
	MX_USB_DEVICE_Init();
	guiData.ptrToUsbBuff = (uint8_t*) usb_rx_buff;
	guiData.ptrToUsbBuffLen = (uint16_t*) &usb_rx_buff_len;
	sprintf(buff, "USB-OTG : Ok");
	SSD1315_DrawString(10, 20, buff);
	SSD1315_UpdateScreen(&hi2c2);

	sprintf(buff, "Servo test: ");
	SSD1315_DrawString(10, 36, buff);
	SSD1315_UpdateScreen(&hi2c2);
	Servo_SetAngle(180);
	HAL_Delay(250);
	Servo_SetAngle(0);
	sprintf(buff, "Servo test: Done");
	SSD1315_DrawString(10, 36, buff);
	SSD1315_UpdateScreen(&hi2c2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM3) {
		trigger_dht_read = true;
		trigger_adc_read = true;
	}
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
