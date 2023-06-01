#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include "gpio.h"
#include "leds.h"
#include "timer.h"
#include "queue.h"
#include "dht11.h"


/*Defining the pins for the SWITCH, LED and the contant ASCII value of 0*/

#define SWITCH 	P_SW
#define LED 		P_LED_R
#define ASCII_0 48


/*Initializing static variables*/

static uint32_t AEM_last_digits;
static int period;
static int seconds;
static int counter = 0;
static int check;
static float temperature;
static float humidity;


/*Timer ISR routine*/

void timer_isr(void){
	
	seconds += 1;
	seconds %= period;

	//Check if |period| seconds have passed
	if (seconds == 0){
		
		// read temperature and humidity from sensor DHT11
		temperature = DHT11_get_temperature(&humidity, &check);
		
		char temp[250];
		sprintf(temp, "The current temperature is: %.2f, humidity is: %.2f and the current period is: %d . Checksum: %d \r\n", temperature, humidity, period, check);
		uart_print(temp);
		
		if (temperature > 25){
			//Turn on LED, if temperature > 25 degrees
			gpio_set(LED, 1);
		}
		if (temperature < 20){
			//Turn off LED, if temperature < 20 degrees
			gpio_set(LED, 0);
		}
	}
	
	//Toggle LED, if temperature in range [20, 25]
	if (temperature <= 25 && temperature >= 20)
		gpio_toggle(LED);
	

}





/*Switch ISR routine*/

void switch_isr(int status){
	//Increase counter, and 'restart' seconds, because period is about to change
	seconds = 0;
	counter ++;
	if(counter == 1){
		//If the switch was pressed once, set period = AEM_last_digits
		period = AEM_last_digits;
	}
	else{
		//If the switch is pressed odd number of timer, period = 3, else set period = 4
		period = (4 - (counter % 2));
		
	}
	
	
	//Print via uart, the counter value and current period
	char temp[200];
	sprintf(temp, "Counter is: %d, new period is: %d \r\n", counter, period);
	uart_print(temp);
}




/*Uart_get_AEM function for obtaining the last 2 digits of user's AEM*/

void uart_get_AEM(void){
	uint8_t c;
	uint8_t c1, c2;
	
	//Initialize queue
	Queue *queue = (Queue*)malloc(15*sizeof(Queue));
	
	queue_init(queue, 15);
	
	c = 'a';
	//Push all the characters the user types, into the queue
	while(c != '\r'){	
		queue_enqueue(queue, c);
		c = uart_rx();
		uart_tx(c);
	}
	uart_print("\r\n");
	
	//Pop one by one the queue items, while the queue is not empty, and keep the last 2 digits in c1, c2
	while(!queue_is_empty(queue)){		
		queue_dequeue(queue, &c1);
		queue_dequeue(queue, &c2);
	}
	
	//Calculate the digits' sum, given that c1, c2 are char type.
	AEM_last_digits = c1 + c2 - 2 * ASCII_0;
	if(AEM_last_digits <= 2){
		AEM_last_digits = 4;
	}
	free(queue);
}




int main(void){
		
	/*setup*/
	
	/*Enable UART*/
	__enable_irq();
	
	uart_init(115200);
	uart_enable();
	
	uart_print("Insert your AEM \r\n");	
	
	/*Initialize LEDs*/
	leds_init();
	
	
	/*Initialize & set mode to SWITCH*/
	gpio_set_callback(SWITCH, switch_isr);
	gpio_set_mode(SWITCH, PullDown);
	gpio_set_trigger(SWITCH, Rising);
	
	
	/*set initial sampling period*/
	period = 2;
	
	
	/*Get user's last 2 digits*/
	uart_get_AEM();

	/*Initiliaze time to 1 sec*/
	timer_init(1e6);
	timer_set_callback(timer_isr);
	timer_enable();
	
	
	/*loop */
	while(1){
			/*Wait for interrupt*/
		__WFI(); 
	}
}