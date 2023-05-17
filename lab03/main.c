#include <stdio.h>
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
float static temperature;

void timer_isr(void){
	seconds += 1;
	seconds %= period;
	if (seconds == 0){
		
		temperature = DHT11_get_temperature();
		char temp[50];
		sprintf(temp, "The current temperature is: %.2f  and the current period is: %d \r\n", temperature, period);
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
	static int counter = 0;
	seconds = 0;
	counter ++;
	if(counter == 1){
		period = AEM_last_digits;
	}
	else{
		period = (4 - (counter % 2));
		
	}
	
}


void uart_get_AEM(void){
	uint8_t c;
	uint8_t c1, c2;
	Queue *queue;
	queue_init(queue, 15);
	
	while((c = uart_rx()) != '\r'){
		queue_enqueue(queue, c);
	}
	
	while(!queue_is_empty(queue)){
		queue_dequeue(queue, &c1);
		queue_dequeue(queue, &c2);
	}
	
	AEM_last_digits = c1 + c2 - 2 * ASCII_0;
	if(AEM_last_digits <= 2){
		AEM_last_digits = 4;
	}
}


int main(void){
		
	/*setup*/
	__enable_irq();
	
	uart_init(115200);
	uart_enable();
	
	uart_print("Insert your AEM \r\n");	
	
	leds_init();
	
	gpio_set_mode(SWITCH, PullDown);
	gpio_set_trigger(SWITCH, Rising);
	
	period = 2;
	timer_init(1e6);
	
	timer_set_callback(timer_isr);

	uart_get_AEM();

	gpio_set_callback(SWITCH, switch_isr);
	timer_enable();
	/*loop */
	while(1){
	
	}
}