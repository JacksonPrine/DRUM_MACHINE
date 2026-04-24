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
void Write_SR_7S(uint8_t temp_Enable, uint8_t temp_Digit);
void Write_7Seg(uint8_t temp_Enable, uint8_t temp_Digit);
void Delay(volatile uint32_t count);
void Init_Timer4(void);
void DAC_Channel1_Init(void);
void DAC_Channel2_Init(void);
void trigger_kick(void);
void trigger_snare(void);
void trigger_hat(void);
void trigger_startup(void);
void TIM4_IRQHandler(void);
void Init_buzzer(void);
//Buzzer Functions for testing. 
void Play_Buzz_Kick(void);
void Play_Buzz_Snare(void);
void Play_Buzz_Hat(void);
void Init_SineTable(void);


volatile uint32_t angle;
volatile uint32_t stepSize;
volatile uint32_t frequency;

#define SAMPLE_RATE 10000.0f
#define TWO_PI 6.2831853f
#define TABLE_SIZE 256
float sine_table[TABLE_SIZE];

// structs for different drum sounds
typedef struct {
    float index;
    float step;
    float env;
    uint8_t active;
} Kick;

typedef struct {
    float index;
    float step;
    float env;
    uint8_t active;
} Startup;

typedef struct {
    float env;
		float hp1;
		float h2;
		float bp;
    uint8_t active;
} NoiseVoice;

// 
volatile Kick kick;
volatile Startup startup;

volatile NoiseVoice snare;
volatile NoiseVoice hat;
volatile float snare_phase = 0.0f;
volatile float hat_phase1 = 0.0f;
volatile float hat_phase2 = 0.0f;
volatile float hat_phase3 = 0.0f;
volatile float hat_lp1 = 0.0f;
volatile float hat_lp2 = 0.0f;

//Noise generator for snare & hi hat
static uint32_t noiseSeed = 1;

//Simple version: Let Kick = 1, Snare = 2, and Hihat = 3.

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
		HAL_Delay(100); 
    SystemClock_Config();
	
		//Turn off all LEDs in Init
		MX_GPIO_Init();
		//Initializing timer 2 to interrupt at desired tempo.
		Init_Timer2();
		//maybe put priority.
	  DAC_Channel1_Init();
    Init_Timer4();
	 Init_SineTable();

		//pattern[0] = 1;
		//pattern[2] = 3;
		//pattern[3] = 1;
		//pattern[4] = 2;
		//pattern[6] = 1;
		//pattern[10] = 1;
		//pattern [12] = 2;
		//pattern [14] = 3;
		//Init_buzzer();
		trigger_startup();

		
    while (1)
    {
    if(Read_Keypad() == 15)
    {
        if(mode == 0) {
            mode = 1;
				}
        else
            mode = 0;

        Delay(10);

        while(Read_Keypad() == 15) {} // wait until key is released

        Delay(10);
    }

			if(mode == 0) {
				if((GPIOB->IDR & (1<<8)) != 0) {
					HAL_Delay(25);
					if(step < 15)
						step++;
					else
						step = 0;
					while((GPIOB->IDR & (1<<8)) != 0)
						{}
					HAL_Delay(25);
				}
				
				if((GPIOB->IDR & (1<<9)) != 0) {
					HAL_Delay(25);
					if(step <= 15)
					{
						pattern[step] = 3;
						step++;
					}
					else
						step = 0;
					while((GPIOB->IDR & (1<<9)) != 0)
						{}
					HAL_Delay(25);
				}
				
				if((GPIOB->IDR & (1<<10)) != 0) 
				{
					HAL_Delay(25);
					if(step <= 15)
					{
						pattern[step] = 2;
						step++;
					}
					else
						step = 0;
					while((GPIOB->IDR & (1<<10)) != 0)
						{}
					HAL_Delay(25);
				}
				
				if((GPIOB->IDR & (1<<11)) != 0) 
				{
					HAL_Delay(25);
					if(step <= 15)
					{
						pattern[step] = 1;
						step++;
					}
					else
						step = 0;
					while((GPIOB->IDR & (1<<11)) != 0)
						{}
					HAL_Delay(25);
				}
				
		}
			//Turn all LEDs off
			GPIOA->ODR &= ~(1<<1);
			GPIOA->ODR &= ~(1<<0);
			GPIOC->ODR &= ~(1<<8);
			GPIOC->ODR &= ~(1<<7);
			
			int LEDtemp = step / 4;
			if(LEDtemp == 0) {
				//Turn on LED 3
				GPIOC->ODR |= (1<<8);
				if(step%4 == 0)		//1.000
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 0); Delay(1);
					Write_7Seg(2, 0); Delay(1);
					Write_SR_7S(0x08, 0x79); Delay(1);
					
					if (mode == 1) {
						if(pattern[0] == 1)
							//Play_Buzz_Kick();
						if(pattern[0] == 2)
							//Play_Buzz_Snare();
						if(pattern[0] == 3) {
							//Play_Buzz_Hat();
					}
					}
				}
				else if(step%4 == 1)//1.250
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 5); Delay(1);
					Write_7Seg(2, 2); Delay(1);
					Write_SR_7S(0x08, 0x79); Delay(1);
					
					if (mode == 1) {
						if(pattern[1] == 1)
						//	Play_Buzz_Kick();
						if(pattern[1] == 2)
							//Play_Buzz_Snare();
						if(pattern[1] == 3) {
						//	Play_Buzz_Hat();
					}
					}
				}
				else if(step%4 == 2)//1.500
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 0); Delay(1);
					Write_7Seg(2, 5); Delay(1);
					Write_SR_7S(0x08, 0x79); Delay(1);
					
					if (mode == 1) {
						if(pattern[2] == 1)
							//Play_Buzz_Kick();
						if(pattern[2] == 2)
							//Play_Buzz_Snare();
						if(pattern[2] == 3) {
							//Play_Buzz_Hat();
						}
				}
				}
				else if(step%4 == 3)//1.750
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 5); Delay(1);
					Write_7Seg(2, 7); Delay(1);
					Write_SR_7S(0x08, 0x79); Delay(1);
					
					if (mode == 1) {
						if(pattern[3] == 1)
							//Play_Buzz_Kick();
						if(pattern[3] == 2)
							//Play_Buzz_Snare();
						if(pattern[3] == 3) {
						//	Play_Buzz_Hat();
						}
					}
				}
			}
			else if (LEDtemp == 1) {
				//Turn on LED 2
				GPIOC->ODR |= (1<<7);
				if(step%4 == 0)//2.000
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 0); Delay(1);
					Write_7Seg(2, 0); Delay(1);
					Write_SR_7S(0x08, 0x24); Delay(1);
					
					if (mode == 1) {
						if(pattern[4] == 1)
						//	Play_Buzz_Kick();
						if(pattern[4] == 2)
							//Play_Buzz_Snare();
						if(pattern[4] == 3) {
						//	Play_Buzz_Hat();
						}
					}
				}
				else if(step%4 == 1)//2.250
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 5); Delay(1);
					Write_7Seg(2, 2); Delay(1);
					Write_SR_7S(0x08, 0x24); Delay(1);
					
					if (mode == 1) {
						if(pattern[5] == 1)
							//Play_Buzz_Kick();
						if(pattern[5] == 2)
							//Play_Buzz_Snare();
						if(pattern[5] == 3) {
							//Play_Buzz_Hat();
						}
					}
				}
				else if(step%4 == 2)//2.500
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 0); Delay(1);
					Write_7Seg(2, 5); Delay(1);
					Write_SR_7S(0x08, 0x24); Delay(1);
					
					if (mode == 1) {
						if(pattern[6] == 1)
							//Play_Buzz_Kick();
						if(pattern[6] == 2)
							//Play_Buzz_Snare();
						if(pattern[6] == 3) {
							//Play_Buzz_Hat();
						}
					}
				}
				else if(step%4 == 3)//2.750
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 5); Delay(1);
					Write_7Seg(2, 7); Delay(1);
					Write_SR_7S(0x08, 0x24); Delay(1);
					
					if (mode == 1) {
						if(pattern[7] == 1)
						//	Play_Buzz_Kick();
						if(pattern[7] == 2)
							//Play_Buzz_Snare();
						if(pattern[7] == 3) {
							//Play_Buzz_Hat();
						}
					}
				}
			}
			else if (LEDtemp == 2) {
				//Turn on LED 1
				GPIOA->ODR |= (1<<0);
				if(step%4 == 0)//3.000
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 0); Delay(1);
					Write_7Seg(2, 0); Delay(1);
					Write_SR_7S(0x08, 0x30); Delay(1);
					
					if (mode == 1) {
						if(pattern[8] == 1)
							//Play_Buzz_Kick();
						if(pattern[8] == 2)
							//Play_Buzz_Snare();
						if(pattern[8] == 3) {
						//	Play_Buzz_Hat();
						}
					}
				}
				else if(step%4 == 1)//3.250
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 5); Delay(1);
					Write_7Seg(2, 2); Delay(1);
					Write_SR_7S(0x08, 0x30); Delay(1);
					
					if (mode == 1) {
						if(pattern[9] == 1)
							//Play_Buzz_Kick();
						if(pattern[9] == 2)
							//Play_Buzz_Snare();
						if(pattern[9] == 3) {
							//Play_Buzz_Hat();
						}
					}
				}
				else if(step%4 == 2)//3.500
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 0); Delay(1);
					Write_7Seg(2, 5); Delay(1);
					Write_SR_7S(0x08, 0x30); Delay(1);
					
					if (mode == 1) {
						if(pattern[10] == 1)
							//Play_Buzz_Kick();
						if(pattern[10] == 2)
							//Play_Buzz_Snare();
						if(pattern[10] == 3) {
							//Play_Buzz_Hat();
						}
					}
				}
				else if(step%4 == 3)//3.750
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 5); Delay(1);
					Write_7Seg(2, 7); Delay(1);
					Write_SR_7S(0x08, 0x30); Delay(1);
					
					if (mode == 1) {
						if(pattern[11] == 1)
							//Play_Buzz_Kick();
						if(pattern[11] == 2)
							//Play_Buzz_Snare();
						if(pattern[11] == 3) {
							//Play_Buzz_Hat();
						}
					}
				}
			}
			else if (LEDtemp == 3) {
				//Turn on LED 0
				GPIOA->ODR |= (1<<1);
				if(step%4 == 0)//4.000
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 0); Delay(1);
					Write_7Seg(2, 0); Delay(1);
					Write_SR_7S(0x08, 0x19); Delay(1);
					
					if (mode == 1) {
						if(pattern[12] == 1)
						//	Play_Buzz_Kick();
						if(pattern[12] == 2)
							//Play_Buzz_Snare();
						if(pattern[12] == 3) {
							//Play_Buzz_Hat();
						}
					}
				}
				else if(step%4 == 1)//4.250
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 5); Delay(1);
					Write_7Seg(2, 2); Delay(1);
					Write_SR_7S(0x08, 0x19); Delay(1);
					
					if (mode == 1) {
						if(pattern[13] == 1)
							//Play_Buzz_Kick();
						if(pattern[13] == 2)
						//	Play_Buzz_Snare();
						if(pattern[13] == 3) {
							//Play_Buzz_Hat();
						}
					}
				}
				else if(step%4 == 2)//4.500
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 0); Delay(1);
					Write_7Seg(2, 5); Delay(1);
					Write_SR_7S(0x08, 0x19); Delay(1);
					
					if (mode == 1) {
						if(pattern[14] == 1)
							//Play_Buzz_Kick();
						if(pattern[14] == 2)
							//Play_Buzz_Snare();
						if(pattern[14] == 3) {
							//Play_Buzz_Hat();
						}
					}
				}
				else if(step%4 == 3)//4.750
				{
					Write_7Seg(4, 0); Delay(1);
					Write_7Seg(3, 5); Delay(1);
					Write_7Seg(2, 7); Delay(1);
					Write_SR_7S(0x08, 0x19); Delay(1);
					
					if (mode == 1) {
						if(pattern[15] == 1)
							//Play_Buzz_Kick();
						if(pattern[15] == 2)
							//Play_Buzz_Snare();
						if(pattern[15] == 3) {
							//Play_Buzz_Hat();
						}
					}
				}
			}
    }
}

void Init_buzzer()
{
uint32_t temp;
RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;		/* enable GPIOC clock */

temp = GPIOC->MODER;
temp &= ~(0x03<<(2*9)); 
temp|=(0x01<<(2*9)); 
GPIOC->MODER = temp;

temp=GPIOC->OTYPER;
temp &=~(0x01<<9); 
GPIOC->OTYPER=temp;

temp=GPIOC->PUPDR;
temp&=~(0x03<<(2*9)); 
GPIOC->PUPDR=temp;
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
      Delay(20);
      LCD_nibble_write(0x30,0);
      Delay(5);
      LCD_nibble_write(0x30,0);
      Delay(1);
      LCD_nibble_write(0x30,0);
      Delay(1);
      LCD_nibble_write(0x20,0);
      Delay(1);
     
           
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

    Delay(25);

    // ---- Scan columns ONCE ----

    GPIOB->ODR &= ~(1<<1);
    GPIOB->ODR &= ~(1<<2);
    GPIOB->ODR &= ~(1<<3);
    GPIOB->ODR &= ~(1<<4);

    // Column 0
    GPIOB->ODR |= (1<<1);
    Delay(2);

    if(GPIOB->IDR & (1<<8)) return 1;
    if(GPIOB->IDR & (1<<9)) return 4;
    if(GPIOB->IDR & (1<<10)) return 7;
    if(GPIOB->IDR & (1<<11)) return 14;

    // Column 1
    GPIOB->ODR &= ~(1<<1);
    GPIOB->ODR |= (1<<2);
    Delay(2);

    if(GPIOB->IDR & (1<<8)) return 2;
    if(GPIOB->IDR & (1<<9)) return 5;
    if(GPIOB->IDR & (1<<10)) return 8;
    if(GPIOB->IDR & (1<<11)) return 0;

    // Column 2
    GPIOB->ODR &= ~(1<<2);
    GPIOB->ODR |= (1<<3);
    Delay(2);

    if(GPIOB->IDR & (1<<8)) return 3;
    if(GPIOB->IDR & (1<<9)) return 6;
    if(GPIOB->IDR & (1<<10)) return 9;
    if(GPIOB->IDR & (1<<11)) return 15;

    // Column 3
    GPIOB->ODR &= ~(1<<3);
    GPIOB->ODR |= (1<<4);
    Delay(2);

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
	TIM2->ARR = 2499; // 10,000/(1249+1) = 8 Hz.
	
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
			if(pattern[step] == 1) trigger_kick();
			if(pattern[step] == 2) trigger_snare();
			if(pattern[step] == 3) trigger_hat();
		}
	}
}

void Write_SR_7S(uint8_t temp_Enable, uint8_t temp_Digit)
{
	int i;
	uint8_t mask=0b10000000;
	for(i=0; i<8; i++)
	{
		if((temp_Digit&mask)==0) 
				GPIOB->ODR&=~(1<<5);
		else
			GPIOB->ODR|=(1<<5);
		/*	Sclck */
		GPIOA->ODR&=~(1<<5);
		Delay(1);
		GPIOA->ODR|=(1<<5);
		Delay(1);
		mask=mask>>1;
	}
	mask=0b10000000;
	for(i=0; i<8; i++)
	{
		if((temp_Enable&mask)==0) 
			GPIOB->ODR&=~(1<<5);
		else
			GPIOB->ODR|=(1<<5);
		/*	Sclck */
		GPIOA->ODR&=~(1<<5);
		/*Delay(1);*/ 
		GPIOA->ODR|=(1<<5);
		/*Delay(1);	*/ 
		mask=mask>>1;
	}
	/*Latch*/
	GPIOC->ODR|=(1<<10); 
	GPIOC->ODR&=~(1<<10);
}


void Write_7Seg(uint8_t temp_Enable, uint8_t temp_Digit)
{
uint8_t Enable[5] = {0x00, 0x08, 0x04, 0x02, 0x01};
/* Enable[i] can enable display i by writing one to DIGIT i and zeros to the other Digits */

	uint8_t Digit[10]= {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};

Write_SR_7S(Enable[temp_Enable], Digit[temp_Digit]);
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

void DAC_Channel1_Init(void)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_DAC1EN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    // PA4 analog mode
    GPIOA->MODER &= ~(3U << (2 * 4));
    GPIOA->MODER |=  (3U << (2 * 4));

    // Disable DAC first
    DAC->CR &= ~(DAC_CR_EN1 | DAC_CR_EN2);

    // Normal mode on channel 1
    DAC->MCR &= ~(7U << 0);

    // No trigger
    DAC->CR &= ~DAC_CR_TEN1;

    // Enable DAC channel 1
    DAC->CR |= DAC_CR_EN1;
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

void Init_Timer4()
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN;

    TIM4->PSC = 39;
    TIM4->ARR = 9;

    TIM4->EGR |= TIM_EGR_UG;

    TIM4->DIER |= TIM_DIER_UIE;

    NVIC_SetPriority(TIM4_IRQn, 1);
    NVIC_EnableIRQ(TIM4_IRQn);

    TIM4->CR1 |= TIM_CR1_CEN;

    TIM4->SR &= ~TIM_SR_UIF;
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

        TIM4->SR &= ~TIM_SR_UIF;

       //Kick
        float kickOut = 0.0f;

		if (kick.active)
		{
				int idx = (int)kick.index & (TABLE_SIZE - 1); // wrap (works because 256)

				kickOut = sine_table[idx] * kick.env;

				kick.index += kick.step;

				// decay
				kick.env *= 0.9995f;

				// pitch drop
				kick.step *= 0.998f;

				if (kick.env < 0.1f)
						kick.active = 0;
		}

       //Snare
        float snareOut = 0.0f;
				static float snare_lp = 0.0f;
				static float hat_lp = 0.0f;
				static float snare_phase = 0.0f;

				if (snare.active)
				{
						float n = noise();

						int idx = (int)snare_phase & (TABLE_SIZE - 1);
						float tone = sine_table[idx];
						snare_phase += (TABLE_SIZE * 1500.0f) / SAMPLE_RATE;

						// Strong initial crack, then short body
						snareOut = ((0.85f * n) + (0.60f * tone)) * snare.env;

						snare.env *= 0.99899997775f;

						if (snare.env < 0.2f)
								snare.active = 0;
				}


				float hatOut = 0.0f;

			if (hat.active)
			{
					float n = noise();

					// Bright hiss: high-pass the noise
					hat_lp1 = 0.82f * hat_lp1 + 0.18f * n;
					float hp = n - hat_lp1;

					// Shape it so it's less like a snare and more like a thin hiss
					hat_lp2 = 0.65f * hat_lp2 + 0.35f * hp;
					float hiss = hp - 0.35f * hat_lp2;

					// Metallic component from 3 pulse-like oscillators
					int i1 = (int)hat_phase1 & (TABLE_SIZE - 1);
					int i2 = (int)hat_phase2 & (TABLE_SIZE - 1);
					int i3 = (int)hat_phase3 & (TABLE_SIZE - 1);

					float p1 = (sine_table[i1] > 0.0f) ? 1.0f : -1.0f;
					float p2 = (sine_table[i2] > 0.0f) ? 1.0f : -1.0f;
					float p3 = (sine_table[i3] > 0.0f) ? 1.0f : -1.0f;

					hat_phase1 += (TABLE_SIZE * 1980.0f) / SAMPLE_RATE;
					hat_phase2 += (TABLE_SIZE * 2710.0f) / SAMPLE_RATE;
					hat_phase3 += (TABLE_SIZE * 3440.0f) / SAMPLE_RATE;

					float metallic = (p1 - p2 + p3) * 0.63f;

					// Very short click at the front
					float clickEnv = hat.env * hat.env * hat.env;
					float click = 0.82f * hp * clickEnv;

					// Main hat body
					hatOut = (1.5f * metallic * hat.env) +
									 (0.74f * hiss * hat.env) +
									 click;

					// Short hat decay
					hat.env *= 0.920f;

					if (hat.env < 0.5f)
							hat.active = 0;
			}
			
			float startOut = 0.0f;
			if (startup.active) 
		{
				int idx = (int)startup.index & (TABLE_SIZE - 1); // wrap (works because 256)

				startOut = sine_table[idx] * startup.env;

				startup.index += startup.step;

				// decay
				startup.env *= 0.99995f;

				// pitch drop
				startup.step *= 1000.0f;

				if (startup.env < 0.1f)
						startup.active = 0;
		}
	

			mix = (kickOut + snareOut + (hatOut*.7f) + startOut) * 1.0f;

			// safety clamp
			if (mix > 1.0f) mix = 1.0f;
			if (mix < -1.0f) mix = -1.0f;

			DAC->DHR12R1 = (uint16_t)((mix * 0.5f + 0.5f) * 4095);
    }
}


void Init_SineTable(void)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        sine_table[i] = sinf((2.0f * 3.14159265f * i) / TABLE_SIZE);
    }
}

void trigger_kick(void)
{
    kick.index = 0;
    kick.step = (TABLE_SIZE * 190.0f) / 25000;
    kick.env = 1.0f;
    kick.active = 1;
}

void trigger_snare(void)
{
    snare.env = 1.0f;
    snare_phase = 0.0f;
    snare.active = 1;
}

void trigger_hat(void)
{
    hat.env = 1.0f;
    hat.active = 1;

    hat_phase1 = 0.0f;
    hat_phase2 = 0.0f;
    hat_phase3 = 0.0f;
    hat_lp1 = 0.0f;
    hat_lp2 = 0.0f;
}

void Play_Buzz_Kick (void) {
		for (int i = 0; i < 50; i++)
		{
		GPIOC->ODR|=(1<<9);
		Delay(4);
		GPIOC->ODR&=~(1<<9);
		Delay(4);
		}
	}
void Play_Buzz_Snare (void) {
		for (int i = 0; i < 50; i++)
		{
		GPIOC->ODR|=(1<<9);
		Delay(3);
		GPIOC->ODR&=~(1<<9);
		Delay(3);
		}
	}

	void trigger_startup(void)
{
    startup.index = 0;
    startup.step = (TABLE_SIZE * 180.0f) / SAMPLE_RATE;
    startup.env = 1.0f;
    startup.active = 1;
}

void Play_Buzz_Hat (void) {
	for (int i = 0; i < 50; i++)
	{
	GPIOC->ODR|=(1<<9);
	Delay(2);
	GPIOC->ODR&=~(1<<9);
	Delay(2);
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

void Delay(volatile uint32_t count)
{
    while (count > 0)
    {
        count--;
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
