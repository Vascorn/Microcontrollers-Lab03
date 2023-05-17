#include <stdio.h>
#include "uart.h"
#include "gpio.h"
#include "leds.h"
#include "timer.h"
#include "queue.h"

#define SWITCH 	P_SW
#define LED 		P_LED_R
#define ASCII_0 48

static uint32_t AEM_last_digits;

void timer_isr(void){
	
}

void switch_isr(int status){
	static int counter = 0;
	
	counter ++;
	if(counter == 1){
		timer_init(AEM_last_digits * 1e6);
	}
	else{
		timer_init( (4 - (counter % 2)) * 1e6);
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
	
	timer_init(2e6);
	
	timer_set_callback(timer_isr);

	uart_get_AEM();

	gpio_set_callback(SWITCH, switch_isr);
	timer_enable();
	/*loop */
	while(1){
	
	}
}