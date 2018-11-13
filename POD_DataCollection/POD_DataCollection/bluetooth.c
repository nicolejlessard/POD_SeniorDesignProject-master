/* 
 * Author: Nicole Lessard
 * Senior Design Project POD
 * Bluetooth Serial Communication Implementation
 */
 
 #include "bluetooth.h"
 #include <avr/io.h>
 
 void bluetooth_init(int baud, int f_cpu){
	 
	 int ubrr = 0;
	 
	 // Calculate UBRR register using baud and f_cpu
	 // For asynchronous normal mode: UBRR = f_cpu/(16*baud) - 1
	 
	 ubrr = f_cpu / (16 * baud);
	 ubrr -= 1;
	 
	 UBRR0H = ubrr >> 8;
	 UBRR0L = ubrr; 
	 
	 // Enable receiver and transmitter
	 UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	 
	 // Set frame format: 8data, 1 stop bit
	 UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	 
 }
 
 char bluetooth_receive(void){
	 
	 while(!(UCSR0A & (1 << UDRE0))); // While buffer is busy, wait
	 
	 return UDR0;
	 
 }
 
 void bluetooth_send(char *str, int length){
	 
	 int x;
	 
	 for(x = 0; x < length; x++){	// While string is not empty
		bluetooth_sendbit(*str);
		str++;	// Increment pointer
	 }
	 
 }
 
 void bluetooth_sendbit(char bit){
	 
	 while(!(UCSR0A & (1 << UDRE0)));	// Wait for buffer
	 
	 UDR0 = bit; 
	 
 }