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
//Hello :33
void SystemClock_Config(void);
void Init_LEDs(void);
static void MX_GPIO_Init(void);
void LCD_nibble_write(uint8_t temp, uint8_t s);
void Write_String_LCD(char *temp);
void Write_Char_LCD(uint8_t code);
void Write_SR_LCD(uint8_t temp);
void Write_Instr_LCD(uint8_t code);
uint8_t Read_Keypad();
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
volatile uint8_t mode = 0;
int main(void)
{
    HAL_Init();
	HAL_Delay(100); 
    SystemClock_Config();
	int last_key = 255;
	int key;
	//Turn off all LEDs in Init
	MX_GPIO_Init();
	//Initializing timer 2 to interrupt at 120 bpm.
	Init_Timer2();
	
    while (1)
    {
			key = Read_Keypad();
			//Turn all LEDs off
			if(key == 15&&last_key!=15)
			{
				GPIOA->ODR &= ~(1<<1);
				GPIOA->ODR &= ~(1<<0);
				GPIOC->ODR &= ~(1<<8);
				GPIOC->ODR &= ~(1<<7);
				mode++;
				if(mode>1)
					mode=0;
			}
			last_key = key;
			
			if(mode == 0)
			{
				
				step =0;
				if(key == 14)
				{
					step++;
					if(step > 15)
						step = 0;
				}

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

			if(mode == 1)
			{
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
}


// Set up the GPIO pins
static void MX_GPIO_Init(void)
{
  // TODO: Initialize any input or output pins you'll use here
     
            uint32_t temp;
RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
      RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
      RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
                 
			//LED0
			temp = GPIOA->MODER;
			temp &= ~(0x03<<(2*1));
			temp|=(0x01<<(2*1));
			GPIOA->MODER = temp;
			temp=GPIOA->OTYPER;
			temp &=~(0x01<<1);
			GPIOA->OTYPER=temp;
			temp=GPIOA->PUPDR;
			temp&=~(0x03<<(2*1));
			GPIOA->PUPDR=temp;
      
				//LED1
			temp = GPIOA->MODER;
			temp &= ~(0x03<<(2*0));
			temp|=(0x01<<(2*0));
			GPIOA->MODER = temp;
			temp=GPIOA->OTYPER;
			temp &=~(0x01<<0);
			GPIOA->OTYPER=temp;
			temp=GPIOA->PUPDR;
			temp&=~(0x03<<(2*0));
			GPIOA->PUPDR=temp;


			//LED2
			temp = GPIOC->MODER;
			temp &= ~(0x03<<(2*7));
			temp|=(0x01<<(2*7));
			GPIOC->MODER = temp;
			temp=GPIOC->OTYPER;
			temp &=~(0x01<<7);
			GPIOC->OTYPER=temp;
			temp=GPIOC->PUPDR;
			temp&=~(0x03<<(2*7));
			GPIOC->PUPDR=temp;

			//LED3
			temp = GPIOC->MODER;
			temp &= ~(0x03<<(2*8));
			temp|=(0x01<<(2*8));
			GPIOC->MODER = temp;
			temp=GPIOC->OTYPER;
			temp &=~(0x01<<8);
			GPIOC->OTYPER=temp;
			temp=GPIOC->PUPDR;
			temp&=~(0x03<<(2*8));
			GPIOC->PUPDR=temp;

     
// Initialization of 7 seg    
      temp = GPIOA->MODER;
      temp &= ~(0x03<<(2*5));
      temp|=(0x01<<(2*5));
      GPIOA->MODER = temp;
                 
      temp=GPIOA->OTYPER;
      temp &=~(0x01<<5);
      GPIOA->OTYPER=temp;
     
      temp=GPIOA->PUPDR;
      temp&=~(0x03<<(2*5));
      GPIOA->PUPDR=temp;

     
      temp = GPIOB->MODER;
      temp &= ~(0x03<<(2*5));
      temp|=(0x01<<(2*5));
      GPIOB->MODER = temp;
                 
      temp=GPIOB->OTYPER;
      temp &=~(0x01<<5);
      GPIOB->OTYPER=temp;
     
      temp=GPIOB->PUPDR;
      temp&=~(0x03<<(2*5));
      GPIOB->PUPDR=temp;
     
      temp = GPIOC->MODER;
      temp &= ~(0x03<<(2*10));
      temp|=(0x01<<(2*10));
      GPIOC->MODER = temp;
                 
      temp=GPIOC->OTYPER;
      temp &=~(0x01<<10);
      GPIOC->OTYPER=temp;
     
      temp=GPIOC->PUPDR;
      temp&=~(0x03<<(2*10));
      GPIOC->PUPDR=temp;      
     
       temp = GPIOA->MODER;
                   temp &= ~(0x03<<(2*10));
                   temp|=(0x01<<(2*10));
                   GPIOA->MODER = temp;
                 
                   temp=GPIOA->OTYPER;
                   temp &=~(0x01<<10);
           GPIOA->OTYPER=temp;
     
                   temp=GPIOA->PUPDR;
           temp&=~(0x03<<(2*10));
           GPIOA->PUPDR=temp;

      /* LCD controller reset sequence */
      HAL_Delay(20);
      LCD_nibble_write(0x30,0);
      HAL_Delay(5);
      LCD_nibble_write(0x30,0);
      HAL_Delay(1);
      LCD_nibble_write(0x30,0);
      HAL_Delay(1);
      LCD_nibble_write(0x20,0);
      HAL_Delay(1);
     
           
      Write_Instr_LCD(0x28); /* set 4 bit data LCD - two line display - 5x8 font*/
      Write_Instr_LCD(0x0E); /* ;turn on display, turn on cursor , turn off blinking            */    
      Write_Instr_LCD(0x01); /* clear display screen and return to home position    */          
      Write_Instr_LCD(0x06); /* ;move cursor to right (entry mode set instruction)        */    

/*configure input*/
      /* row0 to 3 are PB11, PB10, PB9, PB8 */
      temp = GPIOB->MODER;
      temp &= ~(0x03<<(2*11));
      temp &= ~(0x03<<(2*10)); temp &= ~(0x03<<(2*9));
      temp &= ~(0x03<<(2*8));
      GPIOB->MODER = temp;
      temp=GPIOB->OTYPER;
      temp &=~(0x01<<11);
      temp &=~(0x01<<10);
      temp &=~(0x01<<9);
      temp &=~(0x01<<8);
      GPIOB->OTYPER=temp;

      temp=GPIOB->PUPDR;
      temp&=~(0x03<<(2*11));
      temp&=~(0x03<<(2*10));
      temp&=~(0x03<<(2*9));
      temp&=~(0x03<<(2*8));
      GPIOB->PUPDR=temp;

      /* Col 0 to 3 are PB1, PB2, PB3, PB4*/
      /*configure output*/
      temp = GPIOB->MODER;
      temp &= ~(0x03<<(2*1));
      temp|=(0x01<<(2*1));
      temp &= ~(0x03<<(2*2));
      temp|=(0x01<<(2*2));
      temp &= ~(0x03<<(2*3));
      temp|=(0x01<<(2*3));
      temp &= ~(0x03<<(2*4));
      temp|=(0x01<<(2*4));
      GPIOB->MODER = temp;

      temp=GPIOB->OTYPER;
      temp &=~(0x01<<1);
      temp &=~(0x01<<2);
      temp &=~(0x01<<3);
      temp &=~(0x01<<4);
      GPIOB->OTYPER=temp;

      temp=GPIOB->PUPDR;
      temp&=~(0x03<<(2*1));
      temp&=~(0x03<<(2*2));
      temp&=~(0x03<<(2*3));
      temp&=~(0x03<<(2*4));
      GPIOB->PUPDR=temp;


}

void LCD_nibble_write(uint8_t temp, uint8_t s)
{
if (s==0) /*writing instruction*/
{
      temp=temp&0xF0;
      temp=temp|0x02; /*RS=1 (bit 0) for data EN=high (bit1)*/
      Write_SR_LCD(temp);
      temp=temp&0xFD; /*RS=1 (bit 0) for data EN=high (bit1)*/
      Write_SR_LCD(temp);
}

else if (s==1) /*writing data*/
{
  temp=temp&0xF0;
      temp=temp|0x03; /*RS=1 (bit 0) for data EN=high (bit1)*/
      Write_SR_LCD(temp);
      temp=temp&0xFD; /*RS=1 (bit 0) for data EN=high (bit1)*/
      Write_SR_LCD(temp);
}
     
}

void Write_String_LCD(char *temp)
{
int i=0;
while(temp[i]!=0)
{
      Write_Char_LCD(temp[i]);
      i=i+1;
}    
}

void Write_Instr_LCD(uint8_t code)
{
      LCD_nibble_write(code&0xF0,0);
     
      code=code<<4;
      LCD_nibble_write(code,0);
}

void Write_Char_LCD(uint8_t code)
{
     
      LCD_nibble_write(code&0xF0,1);
     
      code=code<<4;
      LCD_nibble_write(code,1);
     
}

uint8_t Read_Keypad()
{
    uint8_t a = 255;

    GPIOB->ODR |= (1<<1)|(1<<2)|(1<<3)|(1<<4);

    // No key pressed ? return immediately
    if((GPIOB->IDR &(0x1<<8))==0 &&
       (GPIOB->IDR &(0x1<<9))==0 &&
       (GPIOB->IDR &(0x1<<10))==0 &&
       (GPIOB->IDR &(0x1<<11))==0)
    {
        return 255;
    }

    HAL_Delay(25);

    // ---- Scan columns ONCE ----

    GPIOB->ODR &= ~(1<<1);
    GPIOB->ODR &= ~(1<<2);
    GPIOB->ODR &= ~(1<<3);
    GPIOB->ODR &= ~(1<<4);

    // Column 0
    GPIOB->ODR |= (1<<1);
    HAL_Delay(2);

    if(GPIOB->IDR & (1<<8)) return 1;
    if(GPIOB->IDR & (1<<9)) return 4;
    if(GPIOB->IDR & (1<<10)) return 7;
    if(GPIOB->IDR & (1<<11)) return 14;

    // Column 1
    GPIOB->ODR &= ~(1<<1);
    GPIOB->ODR |= (1<<2);
    HAL_Delay(2);

    if(GPIOB->IDR & (1<<8)) return 2;
    if(GPIOB->IDR & (1<<9)) return 5;
    if(GPIOB->IDR & (1<<10)) return 8;
    if(GPIOB->IDR & (1<<11)) return 0;

    // Column 2
    GPIOB->ODR &= ~(1<<2);
    GPIOB->ODR |= (1<<3);
    HAL_Delay(2);

    if(GPIOB->IDR & (1<<8)) return 3;
    if(GPIOB->IDR & (1<<9)) return 6;
    if(GPIOB->IDR & (1<<10)) return 9;
    if(GPIOB->IDR & (1<<11)) return 15;

    // Column 3
    GPIOB->ODR &= ~(1<<3);
    GPIOB->ODR |= (1<<4);
    HAL_Delay(2);

    if(GPIOB->IDR & (1<<8)) return 10;
    if(GPIOB->IDR & (1<<9)) return 11;
    if(GPIOB->IDR & (1<<10)) return 12;
    if(GPIOB->IDR & (1<<11)) return 13;

    return 255;
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

void Write_SR_LCD(uint8_t temp)
{
int i;
uint8_t mask=0b10000000;
     
for(i=0; i<8; i++) {
        if((temp&mask)==0)
        GPIOB->ODR&=~(1<<5);
        else
        GPIOB->ODR|=(1<<5);

        /*  Sclck */
        GPIOA->ODR&=~(1<<5); GPIOA->ODR|=(1<<5);
        HAL_Delay(1);

        mask=mask>>1;
        }

    /*Latch*/
    GPIOA->ODR|=(1<<10);
    GPIOA->ODR&=~(1<<10);
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
