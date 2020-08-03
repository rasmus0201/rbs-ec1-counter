#include "mbed.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include <string>

float potentiometerValue = 0;
int buzzerState = 0;
int countIncrement = 10;
int counter = 1;
int counterMax = 10000;
TS_StateTypeDef TS_State;

InterruptIn button(D2);
DigitalOut led(D3);
DigitalOut buzzer(D6);
AnalogIn potentiometer(A0);

void buttonCallback()
{
    if (!TS_State.touchDetected) {
        counter += 100;
    }
}

void buzzerController()
{
    int interval = 5; // 50ms * 5 = 0.25 sec
    int buzzerCounter = interval;
    
    while(true) {
        if (buzzerState == 1) {
            // State have changed to 1
            // There for we should reset buzzerCounter to original interval
            if (buzzerCounter == 0) {
                buzzerCounter = interval;
            }
            
            buzzer.write(1);

            // Decrement counter
            buzzerCounter--;

            // We reached all counts,
            // set state to 0
            if (buzzerCounter == 0) {
                buzzerState = 0;
            }
        }

        if (buzzerState == 0) {
            buzzer.write(0);
        }

        ThisThread::sleep_for(50ms);
    }
}

void potentiometerController()
{
    float inputStart = 0;
    float inputEnd = 1024;
    float outputStart = 0;
    float outputEnd = 100;
    
    while(true) {
        potentiometerValue = potentiometer.read() * 1024;
        countIncrement = int(outputStart + ((outputEnd - outputStart) / (inputEnd - inputStart)) * (potentiometerValue - inputStart));

        ThisThread::sleep_for(100ms);
    }
}

int main()
{
    set_time(0);
    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FB_START_ADDRESS);
    BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);

    uint8_t status = BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
    if (status != TS_OK) {
        BSP_LCD_Clear(LCD_COLOR_RED);
        BSP_LCD_SetBackColor(LCD_COLOR_RED);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN INIT FAIL", CENTER_MODE);

        exit(-1);
    }

    BSP_LCD_Clear(LCD_COLOR_LIGHTYELLOW);
    BSP_LCD_SetBackColor(LCD_COLOR_LIGHTYELLOW);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"Welcome to", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, LINE(6), (uint8_t *)"The advanced counter!", CENTER_MODE);

    HAL_Delay(4000);

    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_Clear(LCD_COLOR_BLACK);

    led.write(0);
    button.rise(&buttonCallback);

    char* displayStr[4];

    Thread buzzerThread;
    buzzerThread.start(&buzzerController);

    Thread potentiometerThread;
    potentiometerThread.start(&potentiometerController);
    
    while(true) {
        BSP_TS_GetState(&TS_State);
        
        sprintf((char *) displayStr, "%d", counter);
        BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *) displayStr, CENTER_MODE);

        if (!TS_State.touchDetected) {
            counter += countIncrement;
            if (counter >= counterMax) {
                BSP_LCD_Clear(LCD_COLOR_BLACK);
                counter = 1;
                buzzerState = 1;
            }
        }

        if (TS_State.touchDetected) {
            led.write(1);
        } else {
            led.write(0);
        }

        HAL_Delay(100);
    }
}
