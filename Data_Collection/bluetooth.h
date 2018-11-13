/* 
 * Author: Nicole Lessard
 * Senior Design Project POD
 * Bluetooth Serial Communication Header
 */
 
 void bluetooth_init(int baud, int f_cpu);
 
 void bluetooth_receive(void);
 
 void bluetooth_send(char *str);
 
 void bluetooth_sendbit(char bit);