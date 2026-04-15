/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "main.h"

void SystemClock_Config(void);
void Init_LEDs(void);
void Init_Timer2(void);

//defining bitfield masks for each drum sound - this allows us to play multiple drum sounds at a single step.
#define KICK_BIT 0b1
#define SNARE_BIT 0b10
#define HIHAT_BIT 0b100
#define OTHER_BIT 0b1000

//globally defining the sequence size as 16
#define BEAT_SIZE 16

uint8_t pattern[BEAT_SIZE];

//notice that the "volatile" keyword is used, since we will very often be editing step's value.
volatile uint8_t step = 0;
//Mode: 0 is Beat Build Mode, 1 is Play mode. Starts on Beat Build Mode.
uint8_t mode = 0;
int main(void)
{
    HAL_Init();
    SystemClock_Config();
	
		//Turn off all LEDs in Init
    Init_LEDs();
		//Initializing timer 2 to interrupt at 120 bpm.
		Init_Timer2();

    while (1)
    {
			
			//Turn all LEDs off
			GPIOA->ODR &= ~(1<<1);
			GPIOA->ODR &= ~(1<<0);
			GPIOC->ODR &= ~(1<<8);
			GPIOC->ODR &= ~(1<<7);
			
			int LEDtemp = step / 4;
			if(LEDtemp == 0) {
				//Turn on LED 3
				GPIOC->ODR |= (1<<8);
			}
			else if (LEDtemp == 1) {
				//Turn on LED 2
				GPIOC->ODR |= (1<<7);
			}
			else if (LEDtemp == 2) {
				//Turn on LED 1
				GPIOA->ODR |= (1<<0);
			}
			else if (LEDtemp == 3) {
				//Turn on LED 0
				GPIOA->ODR |= (1<<1);
			}
    }
}


void Init_LEDs()
{
    uint32_t temp;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

		//Initializing LED0
    temp = GPIOA->MODER;
    temp &= ~(0x03 << (2 * 1));
    temp |= (0x01 << (2 * 1));
    GPIOA->MODER = temp;

    temp = GPIOA->OTYPER;
    temp &= ~(0x01 << 1);
    GPIOA->OTYPER = temp;

    temp = GPIOA->PUPDR;
    temp &= ~(0x03 << (2 * 1));
    GPIOA->PUPDR = temp;
	
		//LED1
    temp = GPIOA->MODER;
    temp &= ~(0x03 << (2 * 0));
    temp |= (0x01 << (2 * 0));
    GPIOA->MODER = temp;

    temp = GPIOA->OTYPER;
    temp &= ~(0x01 << 0);
    GPIOA->OTYPER = temp;

    temp = GPIOA->PUPDR;
    temp &= ~(0x03 << (2 * 0));
    GPIOA->PUPDR = temp;
		
		//LED2
		temp = GPIOC->MODER;
    temp &= ~(0x03 << (2 * 7));
    temp |= (0x01 << (2 * 7));
    GPIOC->MODER = temp;

    temp = GPIOC->OTYPER;
    temp &= ~(0x01 << 7);
    GPIOC->OTYPER = temp;

    temp = GPIOC->PUPDR;
    temp &= ~(0x03 << (2 * 7));
    GPIOC->PUPDR = temp;
		
		//LED3
		temp = GPIOC->MODER;
    temp &= ~(0x03 << (2 * 8));
    temp |= (0x01 << (2 * 8));
    GPIOC->MODER = temp;

    temp = GPIOC->OTYPER;
    temp &= ~(0x01 << 8);
    GPIOC->OTYPER = temp;

    temp = GPIOC->PUPDR;
    temp &= ~(0x03 << (2 * 8));
    GPIOC->PUPDR = temp;
}

void Init_Timer2() {
	//Enabling timer 2 clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	
	//Selecting upcounting mode for Timer2
	TIM2->CR1 &= ~TIM_CR1_DIR;
	
	//Note: We are initalizing PSC and ARR to fit a Beats Per Minute (BPM) of 120. 
	
	//Setting the prescaler, which slows down the input clock by (1 + prescaler)
	TIM2->PSC = 399; //4 MHz (system clock) / 399 + 1 = 10,000 Hz
	
	//Setting the auto-reload, which is how often the the timer2 restarts
	TIM2->ARR = 1249; // 10,000/(1249+1) = 8 Hz.
	
	//This line immediately updates Timer 2's PRC and ARR, preventing anything weird happening during first timer cycle.
	TIM2->EGR |= TIM_EGR_UG;
	
	//Clearing the UIF (Update Interupt Flag). Must be done after
	TIM2->SR &= ~TIM_SR_UIF;
	
	//Enabling update interrupts
	TIM2->DIER |= TIM_DIER_UIE;
	
	//Enabling TIM2 interrupt in NVIC
	NVIC_EnableIRQ(TIM2_IRQn);
																																																							
	//Enabling counter (So timer actually increments)
	TIM2->CR1 |= TIM_CR1_CEN;
	
}

void TIM2_IRQHandler(void) {
	//check if the interrupt flag is set.
	if(TIM2->SR & TIM_SR_UIF) {
		
			//if so, clear it
			TIM2->SR &= ~TIM_SR_UIF;
			//progressing 1 step in the sequence. (if we are in Play Mode).
			if(mode == 1) {
			step++;
			
			//wrapping back to start of sequence.
			if(step >= BEAT_SIZE) {
				step = 0;
			}
			}
	}
}
/* ---------------------- CLOCK CONFIGURATION ---------------------- */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6; // 4 MHz
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|
                                RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
}



// This function is called whenever there is an error, such as illegal memory access
void Error_Handler(void)
{
  // We don't really need to do anything for errors, so just stay here forever
  __disable_irq();
  while (1)
  {
  }
}



// ! IMPORTANT ! All code below this line is generated by STM32CubeMX and should not be modified
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
