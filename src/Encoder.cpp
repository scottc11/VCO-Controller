#include "Encoder.h"

static uint16_t newCount;
static uint16_t prevCount;

// this kinda sorta works
// take from https://os.mbed.com/forum/platform-34-ST-Nucleo-F401RE-community/topic/4963/?page=1#comment-26830
void EncoderInitialiseTIM3(void) {
    // configure GPIO PA0 & PA1 aka A0 & A1 as inputs for Encoder
    // Enable clock for GPIOA
    __GPIOA_CLK_ENABLE(); //equivalent from hal_rcc.h
 
    //stm32f4xx.h 
    GPIOA->MODER   |= GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1 ;           //PA6 & PA7 as Alternate Function   /*!< GPIO port mode register,               Address offset: 0x00      */
    GPIOA->OTYPER  |= GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7 ;                 //PA6 & PA7 as Inputs               /*!< GPIO port output type register,        Address offset: 0x04      */
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7 ;     //Low speed                         /*!< GPIO port output speed register,       Address offset: 0x08      */
    GPIOA->PUPDR   |= GPIO_PUPDR_PUPDR6_1 | GPIO_PUPDR_PUPDR7_1 ;           //Pull Down                         /*!< GPIO port pull-up/pull-down register,  Address offset: 0x0C      */
    GPIOA->AFR[0]  |= 0x22000000 ;                                          //AF02 for PA6 & PA7                /*!< GPIO alternate function registers,     Address offset: 0x20-0x24 */
    GPIOA->AFR[1]  |= 0x00000000 ;                                          //nibbles here refer to gpio8..15   /*!< GPIO alternate function registers,     Address offset: 0x20-0x24 */
   
    // configure TIM3 as Encoder input
    // Enable clock for TIM3
    __TIM3_CLK_ENABLE();
 
    TIM3->CR1   = 0x0001;     // CEN(Counter ENable)='1'     < TIM control register 1
    TIM3->SMCR  = TIM_ENCODERMODE_TI12;     // SMS='011' (Encoder mode 3)  < TIM slave mode control register
    TIM3->CCMR1 = 0xF1F1;     // CC1S='01' CC2S='01'         < TIM capture/compare mode register 1
    TIM3->CCMR2 = 0x0000;     //                             < TIM capture/compare mode register 2
    TIM3->CCER  = 0x0011;     // CC1P CC2P                   < TIM capture/compare enable register
    TIM3->PSC   = 0x0000;     // Prescaler = (0+1)           < TIM prescaler
    TIM3->ARR   = 0xffffffff; // reload at 0xfffffff         < TIM auto-reload register
  
    TIM3->CNT = 0x0000;  //reset the counter before we use it  
}


void EncoderInit(TIM_Encoder_InitTypeDef * encoder, TIM_HandleTypeDef * timer, TIM_TypeDef * TIMx) {
  timer->Instance = TIMx;
  timer->Init.Period = 255;
  timer->Init.CounterMode = TIM_COUNTERMODE_UP;
  timer->Init.Prescaler = 0;
  timer->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

  encoder->EncoderMode = TIM_ENCODERMODE_TI1;

  encoder->IC1Filter = 0x0F;
  encoder->IC1Polarity = TIM_INPUTCHANNELPOLARITY_RISING;
  encoder->IC1Prescaler = TIM_ICPSC_DIV1; // TIM_ICPSC_DIV4;
  encoder->IC1Selection = TIM_ICSELECTION_DIRECTTI;

  encoder->IC2Filter = 0x0F;
  encoder->IC2Polarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  encoder->IC2Prescaler = TIM_ICPSC_DIV1; // TIM_ICPSC_DIV4;
  encoder->IC2Selection = TIM_ICSELECTION_DIRECTTI;


  if (HAL_TIM_Encoder_Init(timer, encoder) != HAL_OK) {
      printf("Couldn't Init Encoder\r\n");
      while (1) {}
  }

  if(HAL_TIM_Encoder_Start(timer, TIM_CHANNEL_ALL)!=HAL_OK) {
      printf("Couldn't Start Encoder\r\n");
      while (1) {}
  }

  __HAL_TIM_ENABLE(timer);
}

uint16_t EncoderRead(TIM_HandleTypeDef *timer) {
  uint16_t val = __HAL_TIM_GET_COUNTER(timer);
  return val;
}

EncoderStatus Encoder_Get_Status(TIM_HandleTypeDef *timer) {
  newCount = EncoderRead(timer);
  if (newCount != prevCount) {
    if (newCount > prevCount) {
      prevCount = newCount;
      return Incremented;
    } else {
      prevCount = newCount;
      return Decremented;
    }
  }
  return Neutral;
}


/*
 * HAL_TIM_Encoder_MspInit()
 * Overrides the __weak function stub in stm32f4xx_hal_tim.h
 *
 * Edit the below for your preferred pin wiring & pullup/down
 * I have encoder common at 3V3, using GPIO_PULLDOWN on inputs.
 * Encoder A&B outputs connected directly to GPIOs.
 *
 * www.st.com/st-web-ui/static/active/en/resource/technical/document/datasheet/DM00102166.pdf
 * www.st.com/st-web-ui/static/active/en/resource/technical/document/datasheet/DM00141306.pdf
 *
 * TIM1_CH1: AF1 @ PA_8, PE_9 
 * TIM1_CH2: AF1 @ PA_9, PE_11 
 *
 * TIM2_CH1: AF1 @ PA_0, PA_5, PA_15, PB_8*     *F446 only
 * TIM2_CH2: AF1 @ PA_1, PB_3, PB_9*            *F446 only
 *
 * TIM3_CH1: AF2 @ PA_6, PB_4, PC_6
 * TIM3_CH2: AF2 @ PA_7, PB_5, PC_7
 *
 * TIM4_CH1: AF2 @ PB_6, PD_12
 * TIM4_CH2: AF2 @ PB_7, PD_13
 *
 * TIM5_CH1: AF2 @ PA_0*    *TIM5 used by mbed system ticker so unavailable
 * TIM5_CH2: AF2 @ PA_1*
 *
 */
 
#ifdef TARGET_STM32F4
void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *htim)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  if (htim->Instance == TIM1) { //PA8 PA9 = Nucleo D7 D8
    __TIM1_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
  else if (htim->Instance == TIM2) { //PA0 PA1 = Nucleo A0 A1
    __TIM2_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
  else if (htim->Instance == TIM3) { //PB4 PB5 = Nucleo D5 D4
    __TIM3_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
  else if (htim->Instance == TIM4) { // PB6 PB7 = Nucleo D10 MORPHO_PB7
    __TIM4_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
}
#endif