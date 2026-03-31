/*
 * Solenoid.C
 *
 *  Created on: Mar 29, 2026
 *      Author: srinimma
 */

#include "solenoid.h"

// GPIO PE15
void actuatePair1(void){
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, GPIO_PIN_SET);
	HAL_Delay(500);	//500 ms
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, GPIO_PIN_RESET);
}

// GPIO PE14
void actuatePair2(void){
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_Delay(500);	//500 ms
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_RESET);
}

// GPIO PE12
void actuatePair3(void){
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_Delay(500);	//500 ms
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_RESET);
}

// GPIO PE10
void actuatePair4void(){
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);
	HAL_Delay(500);	//500 ms
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);
}

// initialize all solenoids to rest state
void Solenoid_init(void) {
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);
}

