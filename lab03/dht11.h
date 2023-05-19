/*!
 * \file      delay.h
 * \brief     Generic delay functions.
 * \copyright ARM University Program &copy; ARM Ltd 2014.
 */
#ifndef DHT11_H
#define DHT11_H

#include <stdint.h>

void DHT11_Start(void);

uint8_t DHT11_Check_Response(void);

uint8_t DHT11_Read(void);

float DHT11_get_temperature(float *hum);


#endif