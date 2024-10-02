/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "rng.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "stm32f7xx_hal.h"
#include "am2320.h"
#include "ILI9341_Touchscreen.h"

#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"

#include <stdio.h>
#include <stdarg.h>
//#include "snow_tiger.h"

#include <toey_resize.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define print(x) HAL_UART_Transmit(&huart3, (uint8_t*)x, strlen(x),1000)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint8_t redVal = 0;
uint8_t greenVal = 0;
uint8_t blueVal = 0;
uint8_t redDecimal = 0;
uint8_t blueDecimal = 0;
uint8_t greenDecimal = 0;
uint8_t someVal = 0;
uint16_t h = 0; //color up to chart
char someValToString[50] = "";
char redPercent[50] = "";
char greenPercent[50] = "";
char bluePercent[50] = "";
uint16_t result = 0;
char resultHex[100] = "";
uint8_t toSecondPage = 0;

float human = 30.0, temp = 40.0;
uint8_t step = 0;
HAL_StatusTypeDef status;

UART_HandleTypeDef huart2;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
uint16_t CRC16_2(uint8_t*, uint8_t);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//void displayHEX(uint32_t myNumber){
//	sprintf(toHex,"0x%08X",myNumber);
//};
uint16_t rgb888_to_rgb565(uint8_t red8, uint8_t green8, uint8_t blue8) {
	// Convert 8-bit red to 5-bit red.
	uint8_t red5 = (uint8_t) ((red8 / 255.0) * 31);
	// Convert 8-bit green to 6-bit green.
	uint8_t green6 = (uint8_t) ((green8 / 255.0) * 63);
	// Convert 8-bit blue to 5-bit blue.
	uint8_t blue5 = (uint8_t) ((blue8 / 255.0) * 31);

	// Shift the red value to the left by 11 bits.
	uint16_t red5_shifted = (uint16_t) (red5) << 11;
	// Shift the green value to the left by 5 bits.
	uint16_t green6_shifted = (uint16_t) (green6) << 5;

	// Combine the red, green, and blue values.
	uint16_t rgb565 = red5_shifted | green6_shifted | blue5;

	return rgb565;
}

void vprint(const char *fmt, va_list argp) {
	char string[200];
	if (0 < vsprintf(string, fmt, argp)) // build string
			{
		HAL_UART_Transmit(&huart2, (uint8_t*) string, strlen(string), 0xffffff); // send message via UART
	}
}

void my_printf(const char *fmt, ...) // custom printf() function
{
	va_list argp;
	va_start(argp, fmt);
	vprint(fmt, argp);
	va_end(argp);
}
float dutyCycleB = 0.25;
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
	char str[50];
	char human[100];
	char temp[100];
	uint8_t cmdBuffer[3];
	uint8_t dataBuffer[8];
	/* USER CODE END 1 */

	/* Enable I-Cache---------------------------------------------------------*/
	SCB_EnableICache();

	/* Enable D-Cache---------------------------------------------------------*/
	SCB_EnableDCache();

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
	MX_USART3_UART_Init();
	MX_SPI5_Init();
	MX_RNG_Init();
	MX_TIM2_Init();
	MX_I2C4_Init();
	MX_ADC1_Init();
	/* USER CODE BEGIN 2 */
	ILI9341_Init(); //initial driver setup to drive ili9341

//	Am2320_HandleTypeDef Am2320_;
//	Am2320_ = am2320_Init(&hi2c4, AM2320_ADDRESS);
	ILI9341_Fill_Screen(WHITE);
	ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
	HAL_ADC_Start(&hadc1);
	int adc_val;
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
//		am2320_GetTemperatureAndHumidity(&Am2320_, &temperature, &humidity);

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

		while (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK) {
		}
		adc_val = HAL_ADC_GetValue(&hadc1);
		if (adc_val <= 340)
			dutyCycleB = 0.25;
		else if (adc_val <= 680)
			dutyCycleB = 0.30;
		else if (adc_val <= 1020)
			dutyCycleB = 0.35;
		else if (adc_val <= 1360)
			dutyCycleB = 0.40;
		else if (adc_val <= 1700)
			dutyCycleB = 0.45;
		else if (adc_val <= 2040)
			dutyCycleB = 0.50;
		else if (adc_val <= 2380)
			dutyCycleB = 0.55;
		else if (adc_val <= 2720)
			dutyCycleB = 0.60;
		else if (adc_val <= 3060)
			dutyCycleB = 0.65;
		else if (adc_val <= 3400)
			dutyCycleB = 0.70;
		else if (adc_val <= 3740)
			dutyCycleB = 0.75;
		else
			dutyCycleB = 0.80;
		htim2.Instance->CCR4 = (1000 - 1) * dutyCycleB;
		HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
		HAL_Delay(100);
		HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_4);
		char str[100];
		sprintf(str,"%.0f%%",dutyCycleB*100,adc_val);
		if(dutyCycleB<=0.45)
		{
			ILI9341_Draw_Text(str, 120, 100,YELLOW , 5, GREEN);
		}
		else if(dutyCycleB<=0.65)
		{
			ILI9341_Draw_Text(str, 120, 100, BLUE, 5, YELLOW);
		}
		else{
			ILI9341_Draw_Text(str, 120, 100, RED, 5, CYAN);
			ILI9341_Draw_Text(str, 120, 100, CYAN, 5, CYAN);
		}

//		//----------------------------------------------------------PERFORMANCE TEST
//		ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1); //�?ลับข้อความ
//		ILI9341_Draw_Text(temp, 30, 30, BLACK, 2, WHITE);
//		ILI9341_Draw_Text(human, 180, 30, BLACK, 2, WHITE);
//		ILI9341_Draw_Filled_Circle(160, 30, 15, h); // x , y , r ,color
//		/////////////////////////////////////////////////////////////////////////
//
//		ILI9341_Draw_Filled_Circle(30, 80, 15, 0XF800);
//		ILI9341_Draw_Rectangle(50, 70, 100, 20, 0XF81F);//x , y , lx, ly ,color
//
//		//ILI9341_Draw_Rectangle(50, 70, 20,20, 0X07E0);
//		if (redVal <= 10) {
//			ILI9341_Draw_Rectangle(50, 70, redVal * 10, 20, 0XF800);//x , y , lx, ly ,color
//			sprintf(redPercent, "%d%%  ", redVal * 10);
//			ILI9341_Draw_Text(redPercent, 180, 70, BLACK, 2, WHITE);
//
//		}
//		////////////////////////////////////green///////////////////////////////////
//		ILI9341_Draw_Filled_Circle(30, 130, 15, 0X07E0);
//		ILI9341_Draw_Rectangle(50, 120, 100, 20, 0XC618);//x , y , lx, ly ,color
//		if (greenVal <= 10) {
//			ILI9341_Draw_Rectangle(50, 120, greenVal * 10, 20, 0X07E0);	//x , y , lx, ly ,color
//			sprintf(greenPercent, "%d%%  ", greenVal * 10);
//			ILI9341_Draw_Text(greenPercent, 180, 120, BLACK, 2, WHITE);
//
//		}
//		////////////////////////////////////blue///////////////////////////////////
//		ILI9341_Draw_Filled_Circle(30, 180, 15, 0X001F);
//		ILI9341_Draw_Rectangle(50, 170, 100, 20, 0X07FF);//x , y , lx, ly ,color
//		if (blueVal <= 10) {
//			ILI9341_Draw_Rectangle(50, 170, blueVal * 10, 20, 0X001F);//x , y , lx, ly ,color
//			sprintf(bluePercent, "%d%%  ", blueVal * 10);
//			ILI9341_Draw_Text(bluePercent, 180, 170, BLACK, 2, WHITE);
//			sprintf(someValToString, "%d", blueDecimal);
//
//		}
//
//		result = rgb888_to_rgb565(redDecimal, greenDecimal, blueDecimal);
//		sprintf(resultHex, "0X%04X", result);
//
//		sscanf(resultHex, "%x", &h);
//
//
//		ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
//		ILI9341_Set_Rotation(SCREEN_VERTICAL_1);
//
//
//		if (TP_Touchpad_Pressed()) {
//
//			uint16_t x_pos = 0;
//			uint16_t y_pos = 0;
//
//			HAL_GPIO_WritePin(GPIOB, LD3_Pin | LD2_Pin, GPIO_PIN_SET);
//
//			uint16_t position_array[2];
//
//			if (TP_Read_Coordinates(position_array) == TOUCHPAD_DATA_OK) {
//				x_pos = position_array[0];
//				y_pos = position_array[1];
//				if ((x_pos >= 146 && x_pos <= 170)
//						&& (y_pos >= 6 && y_pos <= 34))	  		//RedCheck
//						{
//					//ILI9341_Draw_Text("r", 180, 210, BLACK, 2, WHITE);
//					redVal += 1;
//					redDecimal += 25;
//					if (redVal == 11) {
//						redVal = 0;
//						redDecimal = 0;
//					}
//				} else if ((x_pos >= 90 && x_pos <= 119)
//						&& (y_pos >= 9 && y_pos <= 60))	  		//GreenCheck
//						{
//					//ILI9341_Draw_Text("g", 180, 210, BLACK, 2, WHITE);
//					greenVal += 1;
//					greenDecimal += 25;
//					if (greenVal == 11) {
//						greenVal = 0;
//						greenDecimal = 0;
//					}
//				} else if ((x_pos >= 44 && x_pos <= 69)
//						&& (y_pos >= 9 && y_pos <= 42))	  		//GreenCheck
//						{
//					//ILI9341_Draw_Text("b", 180, 210, BLACK, 2, WHITE);
//					blueVal += 1;
//					blueDecimal += 25;
//					if (blueVal == 11) {
//						blueVal = 0;
//						blueDecimal = 0;
//					}
//				} else if ((x_pos >= 200 && x_pos <= 231)
//						&& (y_pos >= 145 && y_pos <= 176))	  		//Center Top
//						{
//					//ILI9341_Draw_Text("c", 180, 210, BLACK, 2, WHITE);
//					toSecondPage = 1;
//				}
//				ILI9341_Draw_Filled_Circle(x_pos, y_pos, 2, BLACK);
//
//				ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
//
//				ILI9341_Set_Rotation(SCREEN_VERTICAL_1);
//			}
//
////	  			  					ILI9341_Draw_Pixel(x_pos, y_pos, BLACK);
//		} else {
//			HAL_GPIO_WritePin(GPIOB, LD3_Pin | LD2_Pin, GPIO_PIN_RESET);
//		}
//
////	  			  		}


	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 144;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 6;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
  /* User can add his own implementation to report the file name and line number,tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
