
#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "spwm.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

volatile uint8_t usb_rx_flag = 0;
volatile uint16_t usb_rx_len = 0;
extern uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

uint32_t custom_atoi(uint8_t *str, uint8_t sz)
{
	uint32_t result = 0;
	uint8_t i = 0;

	while (i < sz) {
			result = result * 10 + (str[i] - '0');
			i++;
	}

	return result;
}

int main(void)
{
	uint8_t *buf = "Hello!";

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();

  spwm_init();

  while (1)
  {
	  if (usb_rx_flag){
	  		uint8_t cmd_str[usb_rx_len + 1];
	  		memcpy(cmd_str, UserRxBufferFS, usb_rx_len);
	  		cmd_str[usb_rx_len] = '\0';

	  		if (!strcmp("start", cmd_str)){
	  				spwm_start();
	  				CDC_Transmit_FS("START--OK", 9);
	  		}

	  		else if (!strcmp("stop", cmd_str)){
	  				spwm_stop();
	  				CDC_Transmit_FS("STOP--OK", 8);
	  		}

	  		else if (!strncmp("set amp ", cmd_str, 8)){
						spwm_set_amplitude(strtof(cmd_str + 8, NULL));
						CDC_Transmit_FS("AMP--OK", 7);
				}

	  		else if (!strncmp("set freq ", cmd_str, 9)){
						spwm_set_freq(strtol(cmd_str + 8, NULL, 10));
						CDC_Transmit_FS("FREQ--OK", 8);
			  }

	  		else if (!strncmp("set ph ", cmd_str, 7)){
						spwm_set_phase(strtol(cmd_str + 7, NULL, 10));
						CDC_Transmit_FS("PH--OK", 6);
				}

	  		else {
	  				CDC_Transmit_FS("UNKNOWN CMD", 11);
	  		}

	  		usb_rx_flag = 0;
	  }
  }

}


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};


  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }


  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
