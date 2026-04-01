/* USER CODE BEGIN Header */

/**

  ******************************************************************************

  * @file           : main.c

  * @brief          : Main program body

  ******************************************************************************

  * @attention

  *

  * Copyright (c) 2026 STMicroelectronics.

  * All rights reserved.

  *

  * This software is licensed under terms that can be found in the LICENSE file

  * in the root directory of this software component.

  * If no LICENSE file comes with this software, it is provided AS-IS.

  *

  ******************************************************************************

  */

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "solenoid.h"
#include <math.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */



/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */



/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim1;
DMA_HandleTypeDef hdma_tim1_ch1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM1_Init(void);
static void MX_SPI3_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */



/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


////////////////////////////////////////////////////////SOLENOID LGOCI/////////////////////////////////////


// GPIO PE15
void actuatePair1(void){ //notes D3, A3, E4, B4
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, GPIO_PIN_SET);
	HAL_Delay(500);	//500 ms
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, GPIO_PIN_RESET);
}

// GPIO PE14
void actuatePair2(void){ //notes Eb3, Bb3, F4, C5
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_Delay(500);	//500 ms
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_RESET);
}

// GPIO PE12
void actuatePair3(void){ //notes E3, B3, F#4, C#5
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_Delay(500);	//500 ms
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_RESET);
}

// GPIO PE10
void actuatePair4(void) { //notes F3, C4, G4, D5
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);
	HAL_Delay(500);	//500 ms
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);
}

// initialize all solenoids to rest state
void Solenoid_init(void) { // notes C3, G3, D4, A4 (open string, no solenoids activated)
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);
}

void Solenoid_Activate(uint8_t midi_note) {
    Solenoid_init(); //turn all off first
    switch(midi_note) {
        case 50: case 57: case 64: case 71: //solenoid 1
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, GPIO_PIN_SET);
            break;
        case 51: case 58: case 65: case 72: //solenoid 2
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_SET);
            break;
        case 52: case 59: case 66: case 73: //solenoid 3
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_SET);
            break;
        case 53: case 60: case 67: case 74: //solenoid 4
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);
            break;
        default: //open string notes, no solenoid needed
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////LED CODE/////////////////////////////////////////////////////
#define MAX_LED 20
#define USE_BRIGHTNESS 1

uint8_t LED_Data[MAX_LED][4];
uint8_t LED_Mod[MAX_LED][4];
int datasentflag = 0;

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
	HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
	datasentflag = 1;
}

void Set_LED(int LED_num, int Red, int Green, int Blue) {
	LED_Data[LED_num][0] = LED_num;
	LED_Data[LED_num][1] = Green;
	LED_Data[LED_num][2] = Red;
	LED_Data[LED_num][3] = Blue;
}

#define PI 3.14159265
void Set_Brightness (int brightness) {
#if USE_BRIGHTNESS
	if (brightness > 45) {
		brightness = 45;
	}
	for (int i = 0; i < MAX_LED; i++) {
		LED_Mod[i][0] = LED_Data[i][0];
		for (int j = 1; j < 4; j++) {
			float angle = 90 - brightness;
			angle = angle * PI / 180;
			LED_Mod[i][j] = (LED_Data[i][j]) / (tan(angle));
		}
	}
#endif
}

uint16_t pwmData[(24 * MAX_LED) + 50];

void WS2812_Send (void) {
	uint32_t idx = 0;
	uint32_t color;
	for (int i= 0; i < MAX_LED; i++) {
#if USE_BRIGHTNESS
		color = ((LED_Mod[i][1] << 16) | (LED_Mod[i][2] << 8) | (LED_Mod[i][3]));
#else
		color = ((LED_Data[i][1] << 16) | (LED_Data[i][2] << 8) | (LED_Data[i][3]));
#endif
		for (int i = 23; i >= 0; i--) {
			if (color&(1 << i)) {
				pwmData[idx] = 60;  // 2/3 of 90
			} else {
				pwmData[idx] = 30;  // 1/3 of 90
			}
			idx++;
		}
	}
	for (int i = 0; i < 50; i++) {
		pwmData[idx] = 0;
		idx++;
	}
	HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)pwmData, idx);
	while (!datasentflag) {

	};
	datasentflag = 0;
}

void Note_On(int note) {
	for (int i = 0; i < MAX_LED; i++) {
		if (i == note) {
			LED_Mod[i][1] = LED_Data[i][1];
            LED_Mod[i][2] = LED_Data[i][2];
            LED_Mod[i][3] = LED_Data[i][3];
        } else {
            LED_Mod[i][1] = 0;
            LED_Mod[i][2] = 0;
            LED_Mod[i][3] = 0;
        }
    }
    WS2812_Send();
}

void Notes_Off(void) {
    for (int i = 0; i < MAX_LED; i++) {
        LED_Mod[i][1] = 0;
        LED_Mod[i][2] = 0;
        LED_Mod[i][3] = 0;
    }

    WS2812_Send();
}
//////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////SPI DISPLAY CODE/////////////////////////////////////////

#define OLED_CS_LOW()   HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET)
#define OLED_CS_HIGH()  HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET)
#define OLED_DC_LOW()   HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET)
#define OLED_DC_HIGH()  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET)
#define OLED_RES_LOW()  HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_RESET)
#define OLED_RES_HIGH() HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_SET)

extern SPI_HandleTypeDef hspi3;

void SSD1306_Command(uint8_t cmd) {
    OLED_DC_LOW();
    OLED_CS_LOW();
    HAL_SPI_Transmit(&hspi3, &cmd, 1, 100);
    OLED_CS_HIGH();
}

void SSD1306_Data(uint8_t *data, uint16_t len) {
    OLED_DC_HIGH();
    OLED_CS_LOW();
    HAL_SPI_Transmit(&hspi3, data, len, 100);
    OLED_CS_HIGH();
}

void SSD1306_Reset(void) {
    OLED_RES_HIGH();
    HAL_Delay(10);
    OLED_RES_LOW();
    HAL_Delay(10);
    OLED_RES_HIGH();
    HAL_Delay(10);
}

void SSD1306_Init(void) {
    SSD1306_Reset();
    HAL_Delay(100);
    SSD1306_Command(0xAE);
    SSD1306_Command(0xD5);
    SSD1306_Command(0x80);
    SSD1306_Command(0xA8);
    SSD1306_Command(0x3F);
    SSD1306_Command(0xD3);
    SSD1306_Command(0x00);
    SSD1306_Command(0x40);
    SSD1306_Command(0x8D);
    SSD1306_Command(0x14);
    SSD1306_Command(0x20);
    SSD1306_Command(0x00);
    SSD1306_Command(0xA1);
    SSD1306_Command(0xC8);
    SSD1306_Command(0xDA);
    SSD1306_Command(0x12);
    SSD1306_Command(0x81);
    SSD1306_Command(0xCF);
    SSD1306_Command(0xD9);
    SSD1306_Command(0xF1);
    SSD1306_Command(0xDB);
    SSD1306_Command(0x40);
    SSD1306_Command(0xA4);
    SSD1306_Command(0xA6);
    SSD1306_Command(0xAF);
}

void SSD1306_Fill(uint8_t pattern) {
    SSD1306_Command(0x21);
    SSD1306_Command(0);
    SSD1306_Command(127);
    SSD1306_Command(0x22);
    SSD1306_Command(0);
    SSD1306_Command(7);

    uint8_t buf[128];
    for (int i = 0; i < 128; i++) buf[i] = pattern;
    for (int page = 0; page < 8; page++) {
        SSD1306_Data(buf, 128);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////MIDI CODE////////////////////////////////////////////////////////////

//MIDI note-to-LED mapping table
//Index = LED index (0 = highest, 19 = lowest)
//Value = MIDI note number
const uint8_t midi_note_map[20] = {
    74, 73, 72, 71, 69, 67, 66, 65, 64, 62,
    60, 59, 58, 57, 55, 53, 52, 51, 50, 48
};

//note name strings for display
const char* note_names[20] = {
    "D5", "C#5", "C5", "B4", "A4",
    "G4", "F#4", "F4", "E4", "D4",
    "C4", "B3", "Bb3", "A3", "G3",
    "F3", "E3", "Eb3", "D3", "C3"
};

//MIDI state machine
volatile uint8_t midi_rx_byte;
volatile uint8_t midi_state = 0;    // 0 = waiting for status, 1 = got status, 2 = got note
volatile uint8_t midi_status = 0;
volatile uint8_t midi_note = 0;
volatile uint8_t midi_velocity = 0;
volatile int8_t  midi_led_index = -1;  // -1 = no note active
volatile uint8_t midi_new_event = 0;   // flag for main loop

//convert MIDI note number to LED index, returns -1 if not mapped
int8_t MIDI_NoteToLED(uint8_t note) {
    for (int i = 0; i < 20; i++) {
        if (midi_note_map[i] == note) return i;
    }
    return -1;
}

//UART receive callback is called every time a byte arrives
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        uint8_t byte = midi_rx_byte;

        if (byte & 0x80) {
            // Status byte
            if ((byte & 0xF0) == 0x90 || (byte & 0xF0) == 0x80) {
                midi_status = byte & 0xF0;
                midi_state = 1;
            } else {
                midi_state = 0;  //ignore other messages
            }
        } else {
            // Data byte
            if (midi_state == 1) {
                midi_note = byte;
                midi_state = 2;
            } else if (midi_state == 2) {
                midi_velocity = byte;
                midi_state = 1;  //running status, ready for next note

                //process message
                if (midi_status == 0x90 && midi_velocity > 0) {
                    //Note ON
                    midi_led_index = MIDI_NoteToLED(midi_note);
                } else {
                    //Note OFF (0x80 or 0x90 with velocity =0)
                    int8_t idx = MIDI_NoteToLED(midi_note);
                    if (idx == midi_led_index) {
                        midi_led_index = -1;
                    }
                }
                midi_new_event = 1;
            }
        }

        //reset for the next byte
        HAL_UART_Receive_IT(&huart2, (uint8_t*)&midi_rx_byte, 1);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////TEXT RENDERING//////////////////////////////////////////////////////

// 5x7 font data, each character is 5 columns of 7 bits
const uint8_t font5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // space
    {0x7C,0x12,0x11,0x12,0x7C}, // A
    {0x7F,0x49,0x49,0x49,0x36}, // B
    {0x3E,0x41,0x41,0x41,0x22}, // C
    {0x7F,0x41,0x41,0x22,0x1C}, // D
    {0x7F,0x49,0x49,0x49,0x41}, // E
    {0x7F,0x09,0x09,0x09,0x01}, // F
    {0x3E,0x41,0x49,0x49,0x7A}, // G
    {0x36,0x49,0x49,0x49,0x36}, // 0
    {0x00,0x42,0x7F,0x40,0x00}, // 1
    {0x42,0x61,0x51,0x49,0x46}, // 2
    {0x21,0x41,0x45,0x4B,0x31}, // 3
    {0x18,0x14,0x12,0x7F,0x10}, // 4
    {0x27,0x45,0x45,0x45,0x39}, // 5
    {0x24,0x7E,0x24,0x7E,0x24}, // sharp symbol
    {0x7F,0x44,0x44,0x44,0x38}, // flat symbol (lowercase b)
};

// converts a character to its font table index
uint8_t char_to_font(char c) {
    switch(c) {
        case ' ': return 0;
        case 'A': return 1;
        case 'B': return 2;
        case 'C': return 3;
        case 'D': return 4;
        case 'E': return 5;
        case 'F': return 6;
        case 'G': return 7;
        case '0': return 8;
        case '1': return 9;
        case '2': return 10;
        case '3': return 11;
        case '4': return 12;
        case '5': return 13;
        case '#': return 14;
        case 'b': return 15;
        default:  return 0;
    }
}

// draws one character scaled up to fill the screen
// for a 2 char string like "D5" each char gets about 60px wide (scale 12x horizontal, 9x vertical)
// for a 3 char string like "C#5" each char gets about 38px wide (scale 7x horizontal, 9x vertical)
// this way the text always fills as much of the 128x64 display as possible
void SSD1306_DrawCharScaled(uint8_t x, uint8_t char_width, uint8_t hscale, char c) {
    uint8_t idx = char_to_font(c);
    const uint8_t *glyph = font5x7[idx];

    // we use all 8 pages (64 pixels tall), scale factor vertically is 64/7 = ~9
    for (uint8_t page = 0; page < 8; page++) {
        uint8_t col_data[128]; // max possible width
        uint8_t col_idx = 0;

        for (uint8_t col = 0; col < 5; col++) {
            uint8_t src = glyph[col];
            uint8_t page_byte = 0;

            // figure out which original row each bit in this page maps to
            for (uint8_t bit = 0; bit < 8; bit++) {
                uint8_t scaled_row = (page * 8) + bit;
                // 64 pixels / 7 rows = 9.14 pixels per row, use *7/64 to map back
                uint8_t orig_row = (scaled_row * 7) / 64;

                if (orig_row < 7 && (src & (1 << orig_row))) {
                    page_byte |= (1 << bit);
                }
            }

            // repeat the column hscale times for horizontal scaling
            for (uint8_t s = 0; s < hscale && col_idx < char_width; s++) {
                col_data[col_idx++] = page_byte;
            }
        }

        SSD1306_Command(0x21);
        SSD1306_Command(x);
        SSD1306_Command(x + char_width - 1);
        SSD1306_Command(0x22);
        SSD1306_Command(page);
        SSD1306_Command(page);
        SSD1306_Data(col_data, char_width);
    }
}

// shows a note name as big as possible on the screen
// calculates the best scale based on how many characters we need to fit
void SSD1306_ShowNote(const char* name) {
    SSD1306_Fill(0x00);

    // count how many characters
    uint8_t len = 0;
    const char* p = name;
    while (*p++) len++;
    if (len == 0) return;

    // gap between characters is 2 pixels
    uint8_t gap = 2;
    // available width for actual characters (subtract gaps between them)
    uint8_t avail = 128 - ((len - 1) * gap);
    // width per character
    uint8_t char_width = avail / len;
    // horizontal scale factor (each original char is 5 cols wide)
    uint8_t hscale = char_width / 5;
    // recalc char_width to be exact multiple of 5
    char_width = hscale * 5;

    // center everything on screen
    uint8_t total_width = (len * char_width) + ((len - 1) * gap);
    uint8_t x_start = (128 - total_width) / 2;

    for (uint8_t i = 0; i < len; i++) {
        SSD1306_DrawCharScaled(x_start + (i * (char_width + gap)), char_width, hscale, name[i]);
    }
}

// clears the display when no note is playing
void SSD1306_ShowEmpty(void) {
    SSD1306_Fill(0x00);
}

////////////////////////////////////////////////////////////////////////////////////////////////




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

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
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_SPI3_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  Set_LED(0,  255, 0,   96);
    Set_LED(1,  255, 0,   64);
    Set_LED(2,  255, 0,   32);
    Set_LED(3,  255, 64,  0);
    Set_LED(4,  255, 128, 0);
    Set_LED(5,  255, 191, 0);
    Set_LED(6,  255, 255, 0);
    Set_LED(7,  191, 255, 0);
    Set_LED(8,  128, 255, 0);
    Set_LED(9,  0,   255, 0);
    Set_LED(10, 0,   255, 128);
    Set_LED(11, 0,   255, 255);
    Set_LED(12, 0,   191, 255);
    Set_LED(13, 0,   128, 255);
    Set_LED(14, 0,   64,  255);
    Set_LED(15, 0,   0,   255);
    Set_LED(16, 64,  0,   255);
    Set_LED(17, 128, 0,   255);
    Set_LED(18, 160, 0,   255);
    Set_LED(19, 192, 0,   255);

    SSD1306_Init();
    SSD1306_Fill(0x00);
    Notes_Off();

    Solenoid_init();

    // Start listening for MIDI
    HAL_UART_Receive_IT(&huart2, (uint8_t*)&midi_rx_byte, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)

  {

      if (midi_new_event) {
          midi_new_event = 0;

          int8_t idx = midi_led_index;
          if (idx >= 0 && idx < 20) {
              Note_On(idx);
              SSD1306_ShowNote(note_names[idx]);
              Solenoid_Activate(midi_note);      // activate the solenoids for the notes
          } else {
              Notes_Off();
              SSD1306_ShowEmpty();
              Solenoid_init();                   //  turn off solenoids
          }
      }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_8;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 18;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 7;
  hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */



  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */



  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 89;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */



  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

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
  huart2.Init.BaudRate = 31250;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
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
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */



  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, OLED_DC_Pin|OLED_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : OLED_DC_Pin OLED_CS_Pin */
  GPIO_InitStruct.Pin = OLED_DC_Pin|OLED_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PE10 PE12 PE14 PE15 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : OLED_RES_Pin */
  GPIO_InitStruct.Pin = OLED_RES_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OLED_RES_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */



  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */



/* USER CODE END 4 */

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
#ifdef USE_FULL_ASSERT
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
