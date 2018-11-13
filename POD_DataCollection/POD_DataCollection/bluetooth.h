/* 
 * Author: Nicole Lessard
 * Senior Design Project POD
 * Bluetooth Serial Communication Header
 */
 
 void bluetooth_init(int baud, int f_cpu);
 
 char bluetooth_receive(void);
 
 void bluetooth_send(char *str, int length);
 
 void bluetooth_sendbit(char bit);