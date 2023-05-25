#include "gpio.h"
#include "delay.h"


#define DHT11_PIN PA_1

void DHT11_Start(void)
{
	gpio_set_mode(DHT11_PIN, Output); // set the pin as output
	gpio_set(DHT11_PIN, 0); // pull the pin low
	delay_us(18000);   // wait for 18ms
	gpio_set_mode(DHT11_PIN, PullUp); // set as input
}

uint8_t DHT11_Check_Response(void)
{
	uint8_t Response = 0;
	delay_us(40);
	if (!(gpio_get(DHT11_PIN)))
	{
		delay_us(80);
		if ((gpio_get(DHT11_PIN))) Response = 1;
		else Response = -1;
	}
	while ((gpio_get(DHT11_PIN)));   // wait for the pin to go low

	return Response;
}

//Read 8 bit data, transmitted from the sensor DHT11
uint8_t DHT11_Read(void)
{
	uint8_t i,j;
	for (j=0;j<8;j++)
	{
		while (!(gpio_get(DHT11_PIN)));   // wait for the pin to go high
		delay_us(40);   // wait for 40 us
		if (!(gpio_get(DHT11_PIN)))   // if the pin is low
		{
			i&= ~(1<<(7-j));   // write 0
		}
		else i|= (1<<(7-j));  // if the pin is high, write 1
		while ((gpio_get(DHT11_PIN)));  // wait for the pin to go low
	}
	return i;
}


//Read temperature & humidity and convert the 2 - 8bit values obtained, to one float.
float DHT11_get_temperature(float *hum){
	DHT11_Start();
	uint8_t Response = DHT11_Check_Response();
	
	uint8_t hd_I, hd_D, t_I, t_D;
	
	hd_I = DHT11_Read();
	hd_D = DHT11_Read();
	t_I = DHT11_Read();
	t_D = DHT11_Read();
	
	//Obtain float value for temperature and humidity
	float temp_float = t_I + t_D/10.0;
	float hum_float = hd_I + hd_D / 10.0;
	
	*hum = hum_float;
	
	
	return temp_float;
}



