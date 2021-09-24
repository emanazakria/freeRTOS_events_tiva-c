#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#define RED       2
#define BLUE      4
#define GREEN     8
#define SWITCHTASKSTACKSIZE        256

EventGroupHandle_t Group1;
volatile uint32_t data1,data2;



void PORTF_Init (void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);    //Enabling PORTF
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);    //Enabling internal LEDs
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_0|GPIO_PIN_4 ); //Enabling switches
}


void UART0_Init (void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);                    //Enabling PORTA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);                    //Enabling UART0

    UARTClockSourceSet(UART0_BASE,UART_CLOCK_SYSTEM);              //UART0 is working using the system clock
    UARTConfigSetExpClk(UART0_BASE,SysCtlClockGet(),115200,(UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE));
    //Setting the UART baudrate, 8 bit mode, one stop bit and no parity bit
    UARTEnable(UART0_BASE);                                       //Enabling UART0
    GPIOPinTypeUART(GPIO_PORTA_BASE,GPIO_PIN_0|GPIO_PIN_1);      //Setting UART0 pins PA0,PA1

}
void Task1(void * para)
{
    uint8_t colors[7]={RED,GREEN,BLUE};   /*LED colors*/
      static  uint8_t i=0;
    while(1)
    {
        xEventGroupWaitBits(Group1,GPIO_PIN_0|GPIO_PIN_4,pdTRUE,pdTRUE,(TickType_t)portMAX_DELAY );
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,colors[i]);
        i= (i+1)%3;
    }
}
void Button1(void * para)
{
    uint32_t flag_low =  GPIO_PIN_4;
    while(1)
    {
        data2= GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);

        if((data2 == 0x00) && (flag_low == GPIO_PIN_4)) //active low , if button is pressed , and last time was high
        {
            xEventGroupSetBits(Group1,GPIO_PIN_4 );
        }

        flag_low = data2;
        vTaskDelay(10);

    }

}
void Button2(void * para)
{  uint32_t flag_low =  GPIO_PIN_0;
while(1)
{
    data1= GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0);

    if((data1 == 0x00) && (flag_low == GPIO_PIN_0)) //active low , if button is pressed , and last time was high
    {
        xEventGroupSetBits(Group1,GPIO_PIN_0);
    }

    flag_low = data1;
    vTaskDelay(10);


}
}
void main(void)
{
    TaskHandle_t  First_handle,Second_handle,Third_handle;
    SysCtlClockSet(SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ|SYSCTL_SYSDIV_2_5); /*80 MHz Clock*/
    PORTF_Init();
    UART0_Init();
    Group1 = xEventGroupCreate();
    xTaskCreate(Button1, "Button1",SWITCHTASKSTACKSIZE, NULL,1, &First_handle);
    xTaskCreate(Button2, "Button2",SWITCHTASKSTACKSIZE, NULL,2, &Second_handle);
    xTaskCreate(Task1, "Task1",SWITCHTASKSTACKSIZE, NULL,3, &Third_handle);

    vTaskStartScheduler();
    while(1)
    {

    }
}
