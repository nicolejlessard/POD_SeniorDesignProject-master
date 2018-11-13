/* 
 * Author: Nicole Lessard
 * Senior Design Project POD
 * Data Collection Code
 */
 
 #ifndef F_CPU 
 #define F_CPU 16000000UL		// Ensure F_CPU is defined as 16 MHz unsigned long
 #endif
 
 #include <avr/io.h>
 #include <util/delay.h>			// Used for delay function
 #include "bluetooth.h"
 
 #define BAUD 9600
 
 #define SparkFun_Power 7		// PD7
 #define SparkFun_Value 0		// PC0
 
 #define DHT_Data 2				// PD2
 #define MAXTIMINGS 85
 #define COUNT 6

 
 int dht_read_data(int pin, float temp, float humidity);
 
 float read_soil(int pin);
 
 void float_to_string(float n, char *str);
 
 int main(void){
	 
	 float temp=0.0, humidity=0.0, moisture=0.0;
	 int minutes = 0, seconds = 0, quarters = 0;
	 char *t = 0, *h = 0, *m = 0;
	 
	 // Initialize SparkFun Power Pin
	 DDRD |= (1 << SparkFun_Power);	// Set pin to output
	 PORTD |= ~(1 << SparkFun_Power); // Ensure SparkFun sensor is off 
	 
	 // Initialize ADC to read SparkFun value
	 ADCSRA |= (1 << ADEN); 		// Enable ADC
	 ADMUX &= 0xF0;					// Clear MUX3..0, When MUX3..0 AC0 is selected for ADC
	 
	 
	 // Initialize DHT22 Data Pin
	 DDRD |= (1 << DHT_Data);		// Set pin to output
	 PORTD |= (1 << DHT_Data);		// Ensure pin is outputing 1
	 
	 bluetooth_init(BAUD, F_CPU);	// Initialize bluetooth
	 
	 while(1){
		 
		 // Clear strings
		 *t = 0;
		 *h = 0;
		 *m = 0;
		 
		// Gather data
		if(dht_read_data(DHT_Data, temp, humidity) < 0){
			
			t = "DHT Error";	// Error message if data is incorrect
			h = "DHT Error";	// Error message if data is incorrect
			
		}
	 
		else{
		 
			float_to_string(temp, t);	// Convert temp and humidity to string
			float_to_string(humidity, h);
		 
		}
	 
		moisture = read_soil(SparkFun_Power);
		float_to_string(moisture, m);	// Convert moisture data to string
	 
		// Send data in format: humidity,temperature,moisture;
		bluetooth_send(h);
		bluetooth_send(",");
		bluetooth_send(t);
		bluetooth_send(",");
		bluetooth_send(m);
		bluetooth_send(";");
		
		// Wait 10 minutes
		
		for(minutes = 0; minutes < 10; minutes++){
			for(seconds = 0; seconds < 60; seconds++){
				for(quarters = 0; quarters < 4; quarters++)
					_delay_ms(250);					// _delay_ms only provides accurate delays up to 262.14 ms after which the resolution drops
			}
		}
	 }
	 
 }
 
 int dht_read_data(int pin, float temp, float humidity){
	 
	 int tmp=0;
	 int sum=0; 
	 int x=0;
	 int y=0;
	 int last_state=1;
	 int counter=0;
	 int data[5] = {0, 0, 0, 0, 0};
	 
	 // Set DHT pin to 1 and wait 250 ms
	 DDRD |= (1 << pin); 		// Ensure pin is output
	 PORTD |= (1 << pin);
	 _delay_ms(250);
	 
	 // Request Reading: 
	 // Pull pin low for 18 ms
	 PORTD &= ~(1 << pin);
	 _delay_ms(18);
	 
	 // Pull pin high for 40 ms
	 PORTD |= (1 << pin);
	 _delay_ms(40);
	 
	 // Set pin to input
	 DDRD &= ~(1 << pin);
	 
	 // Read data
	 for(x = 0; x < MAXTIMINGS; x++){
		 
		 counter = 0;
		 
		 while(1){
			 tmp = ((PIND & (1 << pin)) >> 1);
			 _delay_us(3);
			 
			 if(tmp != last_state) break;
			 
			 counter++; 
			 _delay_us(1);
			 
			 if(counter == 255) break;
		 }
		 
		 last_state = ((PIND & (1 << pin)) >> 1);
		 
		 if(counter == 255) break;
		 
		 // Ignore first three transitions
		 if((i >= 4) && ((i % 2) == 0)) {
			 // Store data
			 data[y/8] <<= 1;
			 if(counter > COUNT) data[y/8] |= 1;
			 
			 y++;
		 }
		 
	 }
	 
	 sum = data[0] + data[1] + data[2] + data[3];		
	 
	 // Convert data into humidity and temperature
	 humidity = data[0] * 256;
	 humidity += data[1];
	 humidity /= 10;
	 
	 temp = (data[2] & 0x7F) * 256;
	 temp += data[3];
	 temp /= 10;
	 
	 if(data[2] & 0x80) temp *= -1; 	// Most Sig bit of data[2] determines sign of temp
	 
	 if((y >= 40) && (data[4] == (sum & 0xFF))) return 0;	// Check if data is good
	 
	 return -1;
	 
 }
 
 float read_soil(int pin){
	 
	 PORTD |= (1 << SparkFun_Power); // Turn on SparkFun sensor
	 
	 ADCSRA |= (1 << ADSC);			// Start ADC conversion
	 
	 while(ADCSRA & (1 << ADSC));	// Wait for conversion to endif
	 
	 return ADC;					// Return converted value
	 
 }
 
 void float_to_string(float n, char *str){
	 
	 int whole = 0;
	 float dec = 0.0;
	 int counter = 0;
	 int i = 0, j = 0;
	 int temp;
	 
	 // Extract integer part of number
	 whole = (int)n;
	 
	 // Extract decimal
	 dec = n - (float)whole;
	 
	 dec *= 1000;	// Up to 3 decimal places will be in string
	 
	 while(dec > 0){	// Add decimal to string first as entire string will be reversed
	 
		str[counter++] = (dec % 10) + '0';
		dec /= 10;
	 
	 }
	 
	 // Add "." to string
	 
	 str[counter++] = '.';
	 
	 while(whole > 0){	// Finally add integer part to string
		
		str[counter++] = (whole % 10) + '0';
		whole /= 10;
	 
	 }
	 
	 j = counter-1;
	 
	 // Reverse string so it is in proper order
	 while(i < j){
		
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++;
		j--;
		
	 }
	 
	 return;
	 
 }