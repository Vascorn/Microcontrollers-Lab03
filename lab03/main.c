#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include "gpio.h"
#include "leds.h"
#include "timer.h"
#include "queue.h"
#include "dht11.h"

#define SWITCH 	P_SW
#define LED 		P_LED_R
#define ASCII_0 48

static uint32_t AEM_last_digits;
static int period;
static int seconds;
static int counter = 0;
static float temperature;
static float humidity;

void timer_isr(void){
	seconds += 1;
	seconds %= period;

	if (seconds == 0){
		
		temperature = DHT11_get_temperature(&humidity);
		
		char temp[250];
		sprintf(temp, "The current temperature is: %.2f, humidity is: %.2f and the current period is: %d \r\n", temperature, humidity, period);
		uart_print(temp);
		
		if (temperature > 25){
			gpio_set(LED, 1);
		}
		if (temperature < 20){
			gpio_set(LED, 0);
		}
	}
	
	if (temperature <= 25 && temperature >= 20)
		gpio_toggle(LED);
	

}

void switch_isr(int status){
	
	
	seconds = 0;
	counter ++;
	if(counter == 1){
		period = AEM_last_digits;
	}
	else{
		period = (4 - (counter % 2));
		
	}
	
	char temp[200];
	sprintf(temp, "Counter is: %d, new period is: %d \r\n", counter, period);
	uart_print(temp);
}


void uart_get_AEM(void){
	uint8_t c;
	uint8_t c1, c2;
	Queue *queue = (Queue*)malloc(15*sizeof(Queue));
	
	queue_init(queue, 15);
	
	c = 'a';
	while(c != '\r'){	
		queue_enqueue(queue, c);
		c = uart_rx();
		uart_tx(c);
	}
	uart_print("\r\n");
	
	while(!queue_is_empty(queue)){		
		queue_dequeue(queue, &c1);
		queue_dequeue(queue, &c2);
	}
	
	AEM_last_digits = c1 + c2 - 2 * ASCII_0;
	if(AEM_last_digits <= 2){
		AEM_last_digits = 4;
	}
	free(queue);
}


int main(void){
		
	/*setup*/
	__enable_irq();
	
	uart_init(115200);
	uart_enable();
	
	uart_print("Insert your AEM \r\n");	
	
	leds_init();
	
	gpio_set_callback(SWITCH, switch_isr);
	gpio_set_mode(SWITCH, PullDown);
	gpio_set_trigger(SWITCH, Rising);
	
	period = 2;
	
	uart_get_AEM();

	
	timer_init(1e6);
	timer_set_callback(timer_isr);
	timer_enable();
	/*loop */
	while(1){
		__WFI();
	}
}