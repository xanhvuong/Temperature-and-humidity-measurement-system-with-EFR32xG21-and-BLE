/***************************************************************************//**
 * @file main.c
 * @brief main() function.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "sl_component_catalog.h"
#include "sl_system_init.h"
#include "app.h"
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_system_kernel.h"
#else // SL_CATALOG_KERNEL_PRESENT
#include "sl_system_process_action.h"
#endif // SL_CATALOG_KERNEL_PRESENT

#include <app_lcd.h>

#include "sl_component_catalog.h"
#include "sl_system_init.h"
#include "app.h"
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_system_kernel.h"
#else // SL_CATALOG_KERNEL_PRESENT
#include "sl_system_process_action.h"
#endif // SL_CATALOG_KERNEL_PRESENT
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"

#include "app_log.h"

#define BSP_TXPORT gpioPortA
#define BSP_RXPORT gpioPortA
#define BSP_TXPIN 5
#define BSP_RXPIN 6
#define BSP_ENABLE_PORT gpioPortD
#define BSP_ENABLE_PIN 4
#define DHT11_PORT gpioPortD
#define DHT11_PIN  3



void initGPIO(void)
{
  GPIO_PinModeSet(BSP_TXPORT, BSP_TXPIN, gpioModePushPull, 1);
  GPIO_PinModeSet(BSP_RXPORT, BSP_RXPIN, gpioModeInput, 0);
  GPIO_PinModeSet(BSP_ENABLE_PORT, BSP_ENABLE_PIN, gpioModePushPull, 1);
}
void initDHT11(void)
{
    GPIO_PinModeSet(DHT11_PORT, DHT11_PIN, gpioModePushPull, 1); // Start DHT11 at High 
}

void delay_us(uint32_t microseconds)
{
    for (uint32_t i = 0; i < (microseconds * 14); i++) {
        __NOP(); // No Operation to wait
    }
}


bool readDHT11(uint8_t *humidity, uint8_t *temperature)
{
    uint8_t data[5] = {0, 0, 0, 0, 0};
    uint32_t timeOut;

    // 1. Send Start signal
    GPIO_PinModeSet(DHT11_PORT, DHT11_PIN, gpioModePushPull, 0); // DHT11 HIGH to LOW
    GPIO_PinOutClear(DHT11_PORT, DHT11_PIN);
    delay_us(18000); // Delay 18ms 
    GPIO_PinOutSet(DHT11_PORT, DHT11_PIN);
    delay_us(20); // Delay 20-40us
    GPIO_PinModeSet(DHT11_PORT, DHT11_PIN, gpioModeInputPull, 1); // Switch to input mode

    // 2. Wait response from DHT11
    timeOut = 0;
    while (GPIO_PinInGet(DHT11_PORT, DHT11_PIN) && timeOut++ < 100) delay_us(1);
    if (timeOut >= 100) return false;

    timeOut = 0;
    while (!GPIO_PinInGet(DHT11_PORT, DHT11_PIN) && timeOut++ < 100) delay_us(1);
    if (timeOut >= 100) return false;

    timeOut = 0;
    while (GPIO_PinInGet(DHT11_PORT, DHT11_PIN) && timeOut++ < 100) delay_us(1);
    if (timeOut >= 100) return false;

    // 3. Read data 40-bit
    for (int i = 0; i < 40; i++)
    {
        // Wait for the first pulse
        timeOut = 0;
        while (!GPIO_PinInGet(DHT11_PORT, DHT11_PIN) && timeOut++ < 100) delay_us(1);
        if (timeOut >= 100) return false;

        delay_us(30); // Delay  to read bit

        if (GPIO_PinInGet(DHT11_PORT, DHT11_PIN)) // if pin at HIGHH
            data[i / 8] |= (1 << (7 - (i % 8)));

        timeOut = 0;
        while (GPIO_PinInGet(DHT11_PORT, DHT11_PIN) && timeOut++ < 100) delay_us(1);
        if (timeOut >= 100) return false;
    }

    // 4. Kiểm tra checksum
    if (data[4] != (data[0] + data[1] + data[2] + data[3]))
        return false;

    *humidity = data[0];
    *temperature = data[2];
    return true;
}



void initUSART0(void)
{
  USART_InitAsync_TypeDef init;
  init.enable = usartEnable;
  init.refFreq = 0;
  init.baudrate = 115200;
  init.oversampling = usartOVS16;
  init.databits = usartDatabits8;
  init.parity = USART_FRAME_PARITY_NONE;
  init.stopbits = usartStopbits1;
  init.mvdis = false;
  init.prsRxEnable = false;
  init.prsRxCh = 0;
  init.autoCsEnable = false;
  init.csInv = false;
  init.autoCsHold = 0;
  init.autoCsSetup = 0;
  init.hwFlowControl = usartHwFlowControlNone;
  GPIO->USARTROUTE[0].TXROUTE = (BSP_TXPORT << _GPIO_USART_TXROUTE_PORT_SHIFT)
            | (BSP_TXPIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].RXROUTE = (BSP_RXPORT << _GPIO_USART_RXROUTE_PORT_SHIFT)
            | (BSP_RXPIN << _GPIO_USART_RXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN | GPIO_USART_ROUTEEN_TXPEN;
  USART_InitAsync(USART0, &init);
}


int main(void)
{
  // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
  // Note that if the kernel is present, processing task(s) will be created by
  // this call.
  sl_system_init();

  // Initialize the application. For example, create periodic timer(s) or
  // task(s) if the kernel is present.
  app_init();


  uint8_t buffer;
    initGPIO();
    initUSART0();

    uint8_t humidity, temperature;
    char uartBuffer[50];


  int count = 0;

#if defined(SL_CATALOG_KERNEL_PRESENT)
  // Start the kernel. Task(s) created in app_init() will start running.
  sl_system_kernel_start();
#else // SL_CATALOG_KERNEL_PRESENT
  while (1) {
    // Do not remove this call: Silicon Labs components process action routine
    // must be called from the super loop.
    sl_system_process_action();

    // Application process.
    app_process_action();


    if (readDHT11(&humidity, &temperature)) {
        //UART
        app_log("Humidity = %d\r\n",humidity);
        app_log("Temperature = %d\r\n",temperature);
        //LCD
        memlcd_app_init(humidity, temperature);
        //BLE
        get_temperature_humidity(humidity, temperature);
    }


#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // Let the CPU go to sleep if the system allows it.
    sl_power_manager_sleep();
#endif
    delay_us(100000); // Delay 2s giữa các lần đọc
  }
#endif // SL_CATALOG_KERNEL_PRESENT
}
