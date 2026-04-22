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
void trigger_snare(void);
void trigger_hat(void);
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
		float hp1;
		float h2;
		float bp;
    uint8_t active;
} NoiseVoice;

// 
volatile Kick kick;

volatile NoiseVoice snare;
volatile NoiseVoice hat;

//Noise generator for snare & hi hat
static uint32_t noiseSeed = 1;
static float hp1 = 0.0f;
static float hp2 = 0.0f;

static float bp = 0.0f;

// simple resonant oscillator state (metallic tone)
static float metal_phase = 0.0f;

int main(void)
{
    HAL_Init();

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    SystemClock_Config();

    Init_LED0();
    DAC_Channel2_Init();
    Init_Timer4();

    while (1)
    {
        trigger_kick();
        //HAL_Delay(500);

        //trigger_snare();
        ///HAL_Delay(500);

        //trigger_hat();
        //HAL_Delay(1000);
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
		float mix;
    if (TIM4->SR & TIM_SR_UIF)
    {
			GPIOA->ODR ^= (1 << 1);
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
            kick.env *= 0.999995f;

            // pitch drop (classic kick behavior)
           // kick.freq = 100.0f;
						
						kick.freq *= 0.998f;

            if (kick.env < 0.001f)
                kick.active = 0;
        }

       //Snare
        float snareOut = 0.0f;

        if (snare.active)
        {
            snareOut = noise() * snare.env;

            snare.env *= 0.99997f;

            if (snare.env < 0.001f)
                snare.active = 0;
        }


				float hatOut = 0.0f;

				if (hat.active)
				{
						// 1. raw noise
						float n = noise();

						// 2. HIGH-PASS (removes low frequencies)
						float hp = n - hp1;
						hp1 = n;

						// 3. SECOND high-pass stage (steeper cutoff = more 808-like)
						float hpB = hp - hp2;
						hp2 = hp;

						// 4. BAND-PASS approximation (difference of two HP stages)
						bp = hpB;

						// 5. metallic resonator (very important for 808 character)
						metal_phase += 0.35f;
						if (metal_phase > TWO_PI) metal_phase -= TWO_PI;

						float metal = sinf(metal_phase);

						// mix noise + metallic tone
						hatOut = (bp * 0.9999f + metal * 0.99999f) * hat.env;

						// 6. fast decay (808 hats are very short)
						hat.env *= 0.9965f;

						if (hat.env < 0.001f)
								hat.active = 0;
				}

			mix = (kickOut + snareOut + hatOut) * 1.0f;

			// safety clamp
			if (mix > 1.0f) mix = 1.0f;
			if (mix < -1.0f) mix = -1.0f;

			DAC->DHR12R2 = (uint16_t)((mix * 0.5f + 0.5f) * 4095);
    }
}

void SystemClock_Config(void)
{
    RCC->CR |= RCC_CR_MSION;
    while (!(RCC->CR & RCC_CR_MSIRDY));

    RCC->CFGR = 0x0;
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

void DAC_Channel2_Init(void)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_DAC1EN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    // PA5 analog mode
    GPIOA->MODER |= (3U << (2 * 5));

    // Disable DAC first
    DAC->CR &= ~(DAC_CR_EN1 | DAC_CR_EN2);

    // Normal mode
    DAC->MCR &= ~(7U << 16);
    DAC->MCR &= ~(7U << 0);

    // No trigger
    DAC->CR &= ~DAC_CR_TEN2;

    // Enable DAC channel 2
    DAC->CR |= DAC_CR_EN2;
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
    hat.env = 0.8f;
    hat.active = 1;

    // reset filter state for consistency
    hp1 = hp2 = 0.0f;
}

void Init_Timer4()
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN;

    TIM4->PSC = 8;
    TIM4->ARR = 9;

    TIM4->EGR |= TIM_EGR_UG;

    TIM4->DIER |= TIM_DIER_UIE;

    NVIC_SetPriority(TIM4_IRQn, 1);
    NVIC_EnableIRQ(TIM4_IRQn);

    TIM4->CR1 |= TIM_CR1_CEN;

    TIM4->SR &= ~TIM_SR_UIF;
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
