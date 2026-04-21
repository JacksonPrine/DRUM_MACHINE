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
#include <math.h>

void Delay(unsigned int ms);
void SystemClock_Config(void);
void Init_LED0(void);
void Init_Timer4(void);
void DAC_Channel2_Init(void);
void trigger_kick(void);
void TIM4_IRQHandler(void);

volatile uint32_t angle;
volatile uint32_t stepSize;
volatile uint32_t frequency;

#define SAMPLE_RATE 44444.0f
#define TWO_PI 6.2831853f

// structs for different drum sounds
typedef struct {
    float phase;
    float freq;
    float env;
    uint8_t active;
} Kick;

typedef struct {
    float env;
    uint8_t active;
} NoiseVoice;

// 
volatile Kick kick;

volatile NoiseVoice snare;
volatile NoiseVoice hat;

//Noise generator for snare & hi hat
static uint32_t noiseSeed = 1;

int main(void)
{
    HAL_Init();

    Init_LED0();
    DAC_Channel2_Init();
	SystemClock_Config();
    Init_Timer4();  

    while (1)
    {
		trigger_kick();
		HAL_Delay(500);
		trigger_snare();
		HAL_Delay(500);
		trigger_hat();
		HAL_Delay(1000);
		trigger_kick();
		trigger_snare();
		HAL_Delay(1000);
    }
}

void Init_LED0()
{
    uint32_t temp;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

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
}

void Delay(unsigned int n)
{
  int i;
  for (; n > 0; n--)
  for (i = 0; i < 136; i++) ;
}

static inline float noise(void)
{
    noiseSeed = 1664525 * noiseSeed + 1013904223;
    return ((noiseSeed >> 16) & 0xFFFF) / 32768.0f - 1.0f;
}


void TIM4_IRQHandler(void)
{
    if (TIM4->SR & TIM_SR_UIF)
    {
        TIM4->SR &= ~TIM_SR_UIF;

       //Kick
        float kickOut = 0.0f;

        if (kick.active)
        {
            kick.phase += TWO_PI * kick.freq / SAMPLE_RATE;

            if (kick.phase > TWO_PI)
                kick.phase -= TWO_PI;

            kickOut = sinf(kick.phase) * kick.env;

            // envelope decay
            kick.env *= 0.995f;

            // pitch drop (classic kick behavior)
            kick.freq *= 0.9995f;

            if (kick.env < 0.001f)
                kick.active = 0;
        }

       //Snare
        float snareOut = 0.0f;

        if (snare.active)
        {
            snareOut = noise() * snare.env;

            snare.env *= 0.96f;

            if (snare.env < 0.001f)
                snare.active = 0;
        }

      //Hihat
        float hatOut = 0.0f;

        if (hat.active)
        {
            hatOut = noise() * hat.env;

            hat.env *= 0.90f;

            if (hat.env < 0.001f)
                hat.active = 0;
        }

      //mixing multiple sounds
        float mix = kickOut + snareOut + hatOut;

        mix *= 0.5f; // prevent clipping

        // DAC output (0–4095)
        DAC->DHR12R2 = (uint16_t)((mix + 1.0f) * 2048.0f);
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

void DAC_Channel2_Init(void) {
	
	//Enabling DAC Clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_DAC1EN;
	
	//Disabling DAC
	DAC->CR &= ~( DAC_CR_EN1 | DAC_CR_EN2 );
	
	//Setting to mode 000
	DAC->MCR &= ~ (7U<<16);
	
	//Disable trigger for DAC channel 2
	DAC->CR &= ~DAC_CR_TEN2;
	
	//Enabling DAC channel 2
	DAC->CR |= DAC_CR_EN2;
	//disable trigger
	DAC->CR &= ~DAC_CR_TEN2;

	//enable output buffer
	DAC->CR &= ~DAC_CR_BOFF2;  
	
	//Enable the clock of GPIO port A 
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
	//Set I/O mode as analog
	GPIOA->MODER &= ~(3U << (2*5));
	GPIOA->MODER |= 3U<<(2*5);
}

void trigger_kick(void)
{
    kick.phase = 0;
    kick.freq = 180.0f;
    kick.env = 1.0f;
    kick.active = 1;
}

void trigger_snare(void)
{
    snare.env = 1.0f;
    snare.active = 1;
}

void trigger_hat(void)
{
    hat.env = 0.6f;
    hat.active = 1;
}

void Init_Timer4() {
	//Enabling timer 4 clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN;
	
	//Selecting upcounting mode for Timer4
	TIM4->CR1 &= ~TIM_CR1_DIR;
	
	//Note: We are initalizing PSC and ARR to fit to a sampling rate of 44.4 kHZ (very close to the common 44.1 kHZ) 
	
	//Setting the prescaler, which slows down the input clock by (1 + prescaler)
	TIM4->PSC = 8; 
	
	//Setting the auto-reload, which is how often the the timer4 restarts
	TIM4->ARR = 9; 
	
	//This line immediately updates Timer 4's PRC and ARR, preventing anything weird happening during first timer cycle.
	TIM4->EGR |= TIM_EGR_UG;
	
	//Clearing the UIF (Update Interupt Flag). Must be done after
	TIM4->SR &= ~TIM_SR_UIF;
	
	//Enabling update interrupts
	TIM4->DIER |= TIM_DIER_UIE;
	
	//Enabling TIM4 interrupt in NVIC
	NVIC_EnableIRQ(TIM4_IRQn);
																																																							
	//Enabling counter (So timer actually increments)
	TIM4->CR1 |= TIM_CR1_CEN;
	
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
