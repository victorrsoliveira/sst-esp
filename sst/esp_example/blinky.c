#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
//#include "user_config.h"
#include "uart.h"
#include "sst_port.h"
#include "sst_exa.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);

static volatile os_timer_t some_timer;
static volatile os_timer_t timer_print;

static SSTEvent tickTaskAQueue[2];
static SSTEvent tickTaskBQueue[2];
static SSTEvent kbdTaskQueue[2];

void SST_start(void) {
}
void SST_onIdle(void) {
	//os_delay_us(10);
	system_soft_wdt_feed();
}

//	Interrupt handler

static void tickISR() {
    uint8_t pin;
    SST_ISR_ENTRY(pin, TICK_ISR_PRIO);

    SST_post(TICK_TASK_A_PRIO, TICK_SIG, 0);     /* post the Tick to Task A */

    SST_ISR_EXIT(pin, 0);
}

static void printISR() {
    uint8_t pin;
    SST_ISR_ENTRY(pin, TICK_ISR_PRIO);

    SST_post(TICK_TASK_B_PRIO, TICK_SIG, 0);     /* post the Tick to Task A */

    SST_ISR_EXIT(pin, 0);
}

//	This is taskA
void some_timerfunc(SSTEvent e)
{
    //Do blinky stuff
    if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & BIT2)
    {
        //Set GPIO2 to LOW
        gpio_output_set(0, BIT2, BIT2, 0);
    }
    else
    {
        //Set GPIO2 to HIGH
        gpio_output_set(BIT2, 0, BIT2, 0);
    }
	
}

//	This is taskB
void print_info(SSTEvent e) {
	os_printf("Hello from ESP8266!\n");
}

//Do nothing function
/*
static void ICACHE_FLASH_ATTR
user_procTask(os_event_t *events)
{
    os_delay_us(10);
}*/

//Init function 
void ICACHE_FLASH_ATTR
user_init()
{
    // Initialize the GPIO subsystem.
    gpio_init();

	// Initialize UART with baud rate of 9600
	uart_init(BIT_RATE_9600, BIT_RATE_9600);

    //Set GPIO2 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

	// Set tick ISR
	//ETS_FRC_TIMER1_INTR_ATTACH(&tickISR,0);

    //Set GPIO2 low
    gpio_output_set(0, BIT2, BIT2, 0);

    //Disarm timer
    os_timer_disarm(&some_timer);
	os_timer_disarm(&timer_print);

    //Setup timer
    os_timer_setfn(&some_timer, (os_timer_func_t *)tickISR, NULL);
	os_timer_setfn(&timer_print, (os_timer_func_t *)printISR, NULL);

    //Arm the timer
    //&some_timer is the pointer
    //1000 is the fire time in ms
    //0 for once and 1 for repeating
    os_timer_arm(&some_timer, 500, 1);
	os_timer_arm(&timer_print, 500, 1);
    os_printf("Hello from ESP8266!\n");
	// Start SST and with tasks

	SST_task(&some_timerfunc, TICK_TASK_A_PRIO,
            tickTaskAQueue, sizeof(tickTaskAQueue)/sizeof(tickTaskAQueue[0]),
            INIT_SIG, 0);

    SST_task(&print_info, TICK_TASK_B_PRIO,
            tickTaskBQueue, sizeof(tickTaskBQueue)/sizeof(tickTaskBQueue[0]),
            INIT_SIG, 0);
	SST_run();
	
    //Start os task
    //system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
}
