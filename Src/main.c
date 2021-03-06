
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 ** This notice applies to any and all portions of this file
 * that are not between comment pairs USER CODE BEGIN and
 * USER CODE END. Other portions of this file, whether
 * inserted by the user or by software development tools
 * are owned by their respective copyright owners.
 *
 * COPYRIGHT(c) 2018 STMicroelectronics
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART6_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

#define RESETPIN 	GPIO_PIN_9			// PC9, Connected to corresponding Reset on SIM800C.

/*!
 * Device states
 */

static enum eState
{
	GSM_START_STATE,
	GSM_SEND_STATE,
	GSM_RESET_STATE,
	GSM_WAIT_STATE
}gsmState;

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

uint8_t rxBuffer[100];
uint8_t txBuffer[100];
uint8_t rxChar;
uint8_t rxCpltFlg;
uint8_t idx = 0;
uint8_t nextTx = 0;
uint8_t len;
//uint8_t txCpltFlg;


/**
 * Section of ATCommands to perform a HTTP GET operation,
 * from "http://m2msupport.net/m2msupport/test.php".
 */
/*
uint8_t atCommandGet[13][100] = {
		"AT\r",
		"AT+CREG?\r",
		"AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r",
		"AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"\r",
		"AT+SAPBR=1,1\r",
		"AT+SAPBR=2,1\r",
		"AT+HTTPINIT\r",
		"AT+HTTPPARA=\"PROIP\",\"0.0.0.0\"\r",
		"AT+HTTPPARA=\"PROPORT\",\"8080\"\r",
		"AT+HTTPPARA=\"CID\",1\r",
		"AT+HTTPPARA=\"URL\",\"http:\/\/m2msupport.net\/m2msupport\/test.php\"\r",
		"AT+HTTPACTION=0\"\r",
		"AT+HTTPREAD\r"			//read the data of the HTTP server
		};
*/
/**
 * HTTP GET operation complete.
 */


/**
 * Section of ATCommands to perform a HTTP POST operation,
 * By Hosting a HTTP Server on "http://139.59.88.117:3250/registerApp".
 * This section of ATCommands will initialize the SIM800C module for -
 * HTTP POST Operation.
 * The content to be hosted is considered to be packed as a JSON object.
 */
uint8_t atCommandStart[12][100] = {
		"AT\r",
		"AT+CREG?\r",										// Get current registration status
		"AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r",				// Set up profile 1, connection type to internet
		"AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"\r",		// Activate bearer profile
		"AT+SAPBR=1,1\r",									// Apply the profile to the connection
		"AT+SAPBR=2,1\r",									// Check if IP is assigned
		"AT+HTTPINIT\r",									// Initiate the HTTP service
		"AT+HTTPPARA=\"PROIP\",\"0.0.0.0\"\r",				// Proxy setting
		"AT+HTTPPARA=\"PROPORT\",\"8080\"\r",				// Proxy setting
		"AT+HTTPPARA=\"CONTENT\",\"application\/json\"\r",	// Set the content type to JSON
		"AT+HTTPPARA=\"CID\",1\r",							// Set the HTTP session
		"AT+HTTPPARA=\"URL\",\"http:\/\/139.59.88.117:3250\/registerApp\"\r"	//Set the HTPP URL
		};
/**
 * HTTP Initialize operation complete.
 */



/**
 * Section of ATCommands to RESET the SIM800C module.
 */
/*
uint8_t atCommandReset[2][15] = {
		"AT+CFUN=0\r",		//Set for minimum functionality
		"AT+CFUN=1\r"		//Set for full functionality
		};
*/
/**
 * SIM800C Resetcomplete.
 */


/**
 * Section of ATCommands to perform a TCP operation
 */
/*
uint8_t atCommandTCP[10][100] = {
		"AT\r",
		"AT+CREG?\r",
		"AT+CGATT=1\r",
		"AT+CIPSHUT\r",
		"AT+CIPMUX=0\r",
		"AT+CSTT=\"airtelgprs.com\",\"\",\"\"\r",
		"AT+CIICR\r",
		"AT+CIFSR\r",
		"AT+CIPSTART=\"TCP\",\"SERVER DOMAIN NAME(or)PUBLIC IP\",\"PORT NUMBER\"\r",
		"AT+CIPSEND\r"
*/
/**
 * TCP operation complete.
 */



/**
 * Section of ATCommands to perform a HTTP POST operation,
 */
uint8_t atCommandPost[5][30] = {
		"AT\r",
		"AT+HTTPTERM\r",			// Terminate the HTTP service
		"AT+HTTPINIT\r",			// Initiate the HTTP service
		"AT+HTTPDATA=50,10000\r",  	// 50 bytes of data, 10sec timeout to enter the data.
									// Inactivity of 10sec will complete data feed operation.
		"AT+HTTPACTION=1\r"			// Start the HTTP POST session
		};
/**
 * HTTP POST operation complete.
 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 *
 * @retval None
 */
int main(void)
{
	/* USER CODE BEGIN 1 */
	uint8_t startIdx = 0;
	uint8_t sendIdx = 0;
//	uint8_t resetIdx = 0;

	/* USER CODE END 1 */

	/* MCU Configuration----------------------------------------------------------*/

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
	MX_USART6_UART_Init();
	/* USER CODE BEGIN 2 */

	gsmState = GSM_START_STATE;
	HAL_UART_Receive_IT(&huart6, &rxChar, 1);
	strcpy(txBuffer, atCommandStart[startIdx]);
	len = strlen(txBuffer);
	HAL_UART_Transmit_IT(&huart6, txBuffer, len);
	startIdx++;
	while(1)
	{
		switch(gsmState){
		case GSM_START_STATE: {
				if(rxCpltFlg==1){
					if(strstr(rxBuffer, "OK")){
						strcpy(txBuffer, atCommandStart[startIdx]);
						len = strlen(txBuffer);
						HAL_UART_Transmit_IT(&huart6, txBuffer, len);
						startIdx++;
						if(startIdx == 13){
							gsmState = GSM_SEND_STATE;
							break;
						}
					}
					else if(strstr(rxBuffer, "ERROR")){
						gsmState = GSM_RESET_STATE;
						break;
					}
					else
						HAL_UART_Receive_IT(&huart6, &rxChar, 1);
					idx = 0;
					rxCpltFlg=0;
					HAL_UART_Receive_IT(&huart6, &rxChar, 1);
					nextTx = 0;

				}
				if(nextTx == 1){
					nextTx = 0;
					HAL_UART_Receive_IT(&huart6, &rxChar, 1);
				}
				break;
			}
		case GSM_RESET_STATE: {
				HAL_GPIO_WritePin(GPIOC, RESETPIN, GPIO_PIN_SET);
				HAL_Delay(1000);
				HAL_GPIO_WritePin(GPIOC, RESETPIN, GPIO_PIN_RESET);
				gsmState = GSM_START_STATE;
				break;
			}
		case GSM_SEND_STATE: {
				if(rxCpltFlg==1){
					if(strstr(rxBuffer, "OK")){
						strcpy(txBuffer, atCommandPost[sendIdx]);
						len = strlen(txBuffer);
						HAL_UART_Transmit_IT(&huart6, txBuffer, len);
						sendIdx++;
//						if(sendIdx == 4){						//
//							--> append JSON data & wait 10sec.	// Accumulated sensor data to be appended here,
//						}										//
						if(sendIdx == 6){
							sendIdx = 0;
							gsmState = GSM_SEND_STATE;
							break;
						}
					}
					else if(strstr(rxBuffer, "ERROR")){
						gsmState = GSM_RESET_STATE;
						break;
					}
					else
						HAL_UART_Receive_IT(&huart6, &rxChar, 1);
					idx = 0;
					rxCpltFlg=0;
					HAL_UART_Receive_IT(&huart6, &rxChar, 1);
					nextTx = 0;

				}
				if(nextTx == 1){
					nextTx = 0;
					HAL_UART_Receive_IT(&huart6, &rxChar, 1);
				}
				break;
			}
		case GSM_WAIT_STATE: {
				HAL_Delay(100);
				break;
			}
		default: {
				gsmState = GSM_START_STATE;
				break;
			}
		}

	}
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

	/**Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/**Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = 16;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 100;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Configure the Systick interrupt time
	 */
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	/**Configure the Systick
	 */
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USART6 init function */
static void MX_USART6_UART_Init(void)
{

	huart6.Instance = USART6;
	huart6.Init.BaudRate = 9600;
	huart6.Init.WordLength = UART_WORDLENGTH_8B;
	huart6.Init.StopBits = UART_STOPBITS_1;
	huart6.Init.Parity = UART_PARITY_NONE;
	huart6.Init.Mode = UART_MODE_TX_RX;
	huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart6.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart6) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

}

/** Configure pins as 
 * Analog
 * Input
 * Output
 * EVENT_OUT
 * EXTI
 */
static void MX_GPIO_Init(void)
{
	  GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);

	  /*Configure GPIO pin : PC9 */
	  GPIO_InitStruct.Pin = GPIO_PIN_9;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  file: The file name as string.
 * @param  line: The line in file as a number.
 * @retval None
 */
void _Error_Handler(char *file, int line)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while(1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

void HAL_UART_RxCpltCallback( UART_HandleTypeDef *huart6 )
{
	if(rxChar == 0x0a){
		rxCpltFlg = 1;
	}
	else{
		rxBuffer[idx++] = rxChar;
		nextTx = 1;
	}
}
//void HAL_UART_TxCpltCallback( UART_HandleTypeDef *huart6 )
//{
//		txCpltFlg = 1;
//}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{ 
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
