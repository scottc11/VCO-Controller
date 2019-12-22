#ifndef __ENCODER_H
#define __ENCODER_H

#include "main.h"

typedef enum {
  Incremented = 1,
  Decremented = -1,
  Neutral = 0,
} EncoderStatus;

void EncoderInit(TIM_Encoder_InitTypeDef * encoder, TIM_HandleTypeDef * timer, TIM_TypeDef * TIMx);
uint16_t EncoderRead(TIM_HandleTypeDef * timer);
void EncoderInitialiseTIM3(void);
EncoderStatus EncoderGetStatus();

#endif