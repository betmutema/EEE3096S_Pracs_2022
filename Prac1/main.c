/* USER CODE BEGIN Header */
/**
*************************************
Info:		STM32 I2C with DS3231 HAL
Author:		Amaan Vally
*************************************
In this practical you will learn to use I2C on the STM32 using the HAL. Here, we will
be interfacing with a DS3231 RTC. We also create functions to convert the data between Binary
Coded Decimal (BCD) and decimal.

Code is also provided to send data from the STM32 to other devices using UART protocol
by using HAL. You will need Putty or a Python script to read from the serial port on your PC.

UART Connections are as follows: red->5V black->GND white(TX)->PA2 green(RX;unused)->PA3.
Open device manager and go to Ports. Plug in the USB connector with the STM powered on. Check the port number (COMx).
Open up Putty and create a new Serial session on that COMx with baud rate of 9600.

https://www.youtube.com/watch?v=EEsI9MxndbU&list=PLfIJKC1ud8ghc4eFhI84z_3p3Ap2MCMV-&index=4

RTC Connections: (+)->5V (-)->GND D->PB7 (I2C1_SDA) C->PB6 (I2C1_SCL)
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hour;
	uint8_t dayofweek;
	uint8_t dayofmonth;
	uint8_t month;
	uint8_t year;
} TIME;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//TO DO:
//TASK 2
//Give DELAY1 and DELAY2 sensible values
#define DELAY1 400
#define DELAY2 23

//TO DO:
//TASK 4
//Define the RTC slave address
#define DS3231_ADDRESS 0x0

#define EPOCH_2022 1640988000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */
char buffer[1000];
char string[60] = { 0 };
uint8_t data [] = "Hello from STM32!\r\n";
TIME time;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void HAL_UART_TxCpltCllback(UART_HandleTypeDef *huart);
void pause_sec(float x);

uint8_t decToBcd(int val);
int bcdToDec(uint8_t val);
void setTime (uint8_t sec, uint8_t min, uint8_t hour, uint8_t dow, uint8_t dom, uint8_t month, uint8_t year);
void getTime (void);
int epochFromTime(TIME time);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Println(UART_HandleTypeDef *uart_handle,char _out[])
{
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
	HAL_UART_Transmit(uart_handle, (uint8_t *) _out,strlen(_out), 60);
	char newline[2] = "\r\n";
	HAL_UART_Transmit(uart_handle, (uint8_t *)newline, 2, 10);

}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void){

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
  MX_I2C1_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */


  //TO DO
  //TASK 6
  //YOUR CODE HERE

  /* USER CODE END 2 */
  setTime(1, 36, 16, 6, 25, 9, 22);

  while (1)
  {
    /* USER CODE END WHILE */
	//TO DO:
	//TASK 1
	//First run this with nothing else in the loop and scope pin PC8 on an oscilloscope
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
	pause_sec(5);// 1s delay for a 60s delay change the value to pause_sec(60)
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);

	//TO DO:
	//TASK 6
	getTime();
	int epTime = epochFromTime(time);
	//The output will be in the format dd-mm-yy , hh:mm:ss , epoch
	sprintf(string, "%02d-%02d-20%02d , %02d:%02d:%02d  , %02d ", time.dayofmonth, time.month, time.year, time.hour, time.minutes, time.seconds, epTime );
	Println(&huart2, string);//

	//This creates a string "55555555555555" with a pointer called buffer
	//Transmit data via UART
	//Blocking! fine for small buffers
	//HAL_UART_Transmit(&huart2, buffer2, sizeof(buffer2), 1000);

  }

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x2000090E;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LD4_Pin|LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD4_Pin LD3_Pin */
  GPIO_InitStruct.Pin = LD4_Pin|LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void pause_sec(float x)
{
	/* Delay program execution for x seconds */
	//TO DO:
	//TASK 2
	//Make sure you've defined DELAY1 and DELAY2 in the private define section

	int count = ((DELAY1*DELAY2)*48)/4;
	for(int i = 0;i < x*count; i++){
	}

	//YOUR CODE HERE
}


uint8_t decToBcd(int val)
{
    //Convert normal decimal numbers to binary coded decimal
	//TO DO:
	//TASK 3
	//char newline[2] = "\r\n";

	return (uint8_t)( (val/10*16) + (val%10) );

	//YOUR CODE HERE
}

int bcdToDec(uint8_t val)
{
    //Convert binary coded decimal to normal decimal numbers
	//TO DO:
	//TASK 3
	//Complete the BCD to decimal function
	//char newline[2] = "\r\n";

	return (int)( (val/16*10) + (val%16) );

	//YOUR CODE HERE

}

void setTime (uint8_t sec, uint8_t min, uint8_t hour, uint8_t dow, uint8_t dom, uint8_t month, uint8_t year)
{
    //Write the time to the RTC using I2C
	//TO DO:
	//TASK 4

	uint8_t set_time[7];

	/* from link https://controllerstech.com/ds3231-rtc-module-with-stm32 */
	//YOUR CODE HERE

	//fill in the address of the RTC, the address of the first register to write anmd the size of each register
	//The function and RTC supports multiwrite. That means we can give the function a buffer and first address
	//and it will write 1 byte of data, increment the register address, write another byte and so on
	set_time[0] = decToBcd(sec);
	sprintf(string, "%02d", set_time[0]);
	Println(&huart2, string);
	set_time[1] = decToBcd(min);
	sprintf(string, "%02d", set_time[1]);
	Println(&huart2, string);
	set_time[2] = decToBcd(hour);
	sprintf(string, "%02d", set_time[2]);
	Println(&huart2, string);
	set_time[3] = decToBcd(dow);
	sprintf(string, "%d", set_time[3]);
	Println(&huart2, string);
	set_time[4] = decToBcd(dom);
	sprintf(string, "%d", set_time[4]);
	Println(&huart2, string);
	set_time[5] = decToBcd(month);
	sprintf(string, "%d", set_time[5]);
	Println(&huart2, string);
	set_time[6] = decToBcd(year);
	sprintf(string, "%d", set_time[6]);
	Println(&huart2, string);

	HAL_I2C_Mem_Write(&hi2c1, DS3231_ADDRESS, 0x00, 1, set_time, 7, 1000);
}

void getTime (void)
{
    //Get the time from the RTC using I2C
	//TO DO:
	//TASK 4
	//Update the global TIME time structure

	uint8_t get_time[7];

	//fill in the address of the RTC, the address of the first register to write anmd the size of each register
	//The function and RTC supports multiread. That means we can give the function a buffer and first address
	//and it will read 1 byte of data, increment the register address, write another byte and so on
	//YOUR CODE HERE

	HAL_I2C_Mem_Read(&hi2c1, DS3231_ADDRESS, 0x00, 1, get_time, 7, 1000);
	time.seconds = bcdToDec(get_time[0]);
	time.minutes = bcdToDec(get_time[1]);
	time.hour = bcdToDec(get_time[2]);
	time.dayofweek = bcdToDec(get_time[3]);
	time.dayofmonth = bcdToDec(get_time[4]);
	time.month = bcdToDec(get_time[5]);
	time.year = bcdToDec(get_time[6]);
}

int epochFromTime(TIME time){
    //Convert time to UNIX epoch time
	//TO DO:
	//TASK 5
	//You have been given the epoch time for Saturday, January 1, 2022 12:00:00 AM GMT+02:00
	//It is define above as EPOCH_2022. You can work from that and ignore the effects of leap years/seconds

	//YOUR CODE HERE
	int no_days = 0;

	switch(time.month){

	case 1:
		no_days += 31;
		break;

	case 2:
		no_days += 31 + 28;
		break;

	case 3:
		no_days += 31 + 28 + 31;
		break;

	case 4:
		no_days += 31 + 28 + 31 + 30;
		break;

	case 5:
		no_days += 31 + 28 + 31 + 30 + 31;
		break;

	case 6:
		no_days += 31 + 28 + 31 + 30 + 31 + 30;
		break;

	case 7:
		no_days += 31 + 28 + 31 + 30 + 31 + 30 + 31;
		break;

	case 8:
		no_days += 31 + 28 + 31 + 30 + 31 + 30 + 31 + 30;
		break;

	case 9:
		no_days += 31 + 28 + 31 + 30 + 31 + 30 + 31 + 30 + 31;
		break;

	case 10:
		no_days += 31 + 28 + 31 + 30 + 31 + 30 + 31 + 30 + 31 + 30;
		break;

	case 11:
		no_days += 31 + 28 + 31 + 30 + 31 + 30 + 31 + 30 + 31 + 30 + 31;
		break;
	}

		return time.seconds+ time.minutes*60 + time.hour*3600 + (time.year -22)*365*24*60*60 + (time.dayofmonth + no_days)*24*60*60 ;
	}

/*USER CODE END 4 */

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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
