#ifndef __SERIAL_H__
#define __SERIAL_H__

#define EX_UART_NUM UART_NUM_2
//#define EX_UART_NUM UART_NUM_0
#define UART_RX_PIN 22
//#define UART_RX_PIN UART_PIN_NO_CHANGE
#define RD_BUF_SIZE 8192

void init_serial();

#endif
