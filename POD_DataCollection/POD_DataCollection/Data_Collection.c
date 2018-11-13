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
 
 #define BAUD 9600
 #define FOSC 16000000
 #define MYUBRR FOSC/BAUD/16 - 1
 
 #define SparkFun_Power 7		// PD7
 #define SparkFun_Value 0		// PC0
 
 #define DHT_Data 2				// PD2
 #define MAXTIMINGS 85
 #define COUNT 6
 
 void serial_init(void);
 
 void serial_sendbit(char bit);
 
 int dht_read_data(int pin, float temp, float humidity);
 
 float read_soil(int pin);
 
 void send_data(float n);
 
 void send_error(void);
 
 int main(void){
	 
	 float temp=0.0, humidity=0.0, moisture=0.0;
	 int seconds = 0, quarters = 0;
	 
	  // Initialize DHT22 Data Pin
	  DDRD |= (1 << DHT_Data);		// Set pin to output
	  PORTD |= (1 << DHT_Data);		// Ensure pin is outputing 1
	 
	 // Initialize SparkFun Power Pin
	 DDRD |= (1 << SparkFun_Power);	// Set pin to output
	 PORTD &= ~(1 << SparkFun_Power); // Ensure SparkFun sensor is off 
	 
	 // Initialize ADC to read SparkFun value
	 ADCSRA |= (1 << ADEN); 		// Enable ADC
	 ADMUX &= ~0xF0;					// Clear MUX3..0, When MUX3..0 AC0 is selected for ADC
	 
	 // Initialize serial communication for bluetooth
	 serial_init();
	 
	 while(1){
		 
		 // Gather data
		 if(dht_read_data(DHT_Data, temp, humidity) < 0){
			 
			 send_error();
			 serial_sendbit(',');
			 send_error();
			 serial_sendbit(',');
			 
		 }
		 
		 else{
			 
			 send_data(temp);
			 serial_sendbit(',');
			 send_data(humidity);
			 serial_sendbit(',');
			 
		 }
		 
		 moisture = read_soil(SparkFun_Power);
		 send_data(moisture);
		 serial_sendbit(';');
		 
		 for(seconds = 0; seconds < 60; seconds++){
			 for(quarters = 0; quarters < 4; quarters++)
				 _delay_ms(250);					// _delay_ms only provides accurate delays up to 262.14 ms after which the resolution drops
		 }
	 }
	 
 }
 
 void serial_init(void){
	 UBRR0H  = (MYUBRR >> 8);
	 UBRR0L  = MYUBRR;
	 
	 UCSR0B |= (1<<RXEN0)  | (1<<TXEN0);
	 UCSR0C |= (1<<UCSZ00) | (1<<UCSZ01);
 }
 
 void serial_sendbit(char bit){
	 while( ( UCSR0A & ( 1 << UDRE0 ) ) == 0 );
	 UDR0 = bit;
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
		 if((y >= 4) && ((y % 2) == 0)) {
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
 
 void send_data(float n){
	 int whole = 0;
	 float dec = 0.0;
	 
	 // If negative, send negative sign
	 if(n < 0) serial_sendbit('-');
	 
	 // Send whole part of number
	 whole = (int)n;
	 
	 // Extract decimal
	 dec = (n - whole) * 1000;		// Up to 3 decimal places will be in string
	 
	 // Send integer part of number
	 if(whole >= 100){
		 whole = whole % 1000;
		 
		 serial_sendbit(whole/100 + '0');
	 }
	 
	 if(whole >= 10){
		 whole = whole % 100;
		 
		 serial_sendbit(whole/10 + '0');
	 }
	 
	whole = whole % 10;
		 
	serial_sendbit(whole + '0');
	 	 
	 serial_sendbit('.');
	 
	 // Send decimal part of number
	 if(dec >= 100){
		 dec = (int) dec % 1000;
		 
		 serial_sendbit(dec/100 + '0');
	 }
	 
	 if(dec >= 10){
		 dec = (int) dec % 100;
		 
		 serial_sendbit(dec/10 + '0');
	 }
	
	dec = (int) dec % 10;
		 
	serial_sendbit(dec + '0');
	 
	 return;
 }
 
 void send_error(void){
	 char error[12] = "Sensor Error";
	 int x;
	 
	 for(x = 0; x < 12; x++){
		 serial_sendbit(error[x]);
	 }
 }
