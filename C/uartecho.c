/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== uartecho.c ========
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>

/* Example/Board Header files */
#include "Board.h"

/* UART Packet */
const char* packet_start = "S";
const char* packet_end = "E";

/* DNN model */
extern int init_model();
extern int predict(float *value_buf, int buf_size);

/* Value buffer (60s) */
#define VALUE_CHAR_SIZE 20
#define VALUE_BUF_SIZE 60
float* value_buf;

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    char        input;
    const char  echoPrompt[] = "Echoing characters:\r\n";
    UART_Handle uart;
    UART_Params uartParams;

    /* Call driver init functions */
    GPIO_init();
    UART_init();

    /* Configure the LED pin */
    GPIO_setConfig(Board_GPIO_LED0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Turn on user LED */
    GPIO_write(Board_GPIO_LED0, Board_GPIO_LED_ON);

    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 115200;

    uart = UART_open(Board_UART0, &uartParams);

    if (uart == NULL) {
        /* UART_open() failed */
        while (1);
    }

    UART_write(uart, echoPrompt, sizeof(echoPrompt));

    /* Initialize DNN */
    init_model();

    /* Initialize value buffer */
    value_buf = (float *) malloc(VALUE_BUF_SIZE * sizeof(float));
    memset(value_buf, 0, VALUE_BUF_SIZE * sizeof(float));
    int value_buf_num = 0;  // predict only when value_buf_num reaches VALUE_BUF_SIZ

    /* Loop forever echoing */
    int idx = 0;
    char value_str[VALUE_CHAR_SIZE];
    float value = 0;

    while (1) {
        UART_read(uart, &input, 1);
        switch(input) {
        case 0: continue;
        case 'S':
            // Transfer value_str to float value
            sscanf(value_str, "%f", &value);

            // Save value in value_buf
            int i;
            for(i = 1; i < value_buf_num; i++) {
                value_buf[i] = value_buf[i - 1];
            }
            value_buf[0] = value;
            value_buf_num += 1;

            // Predict when meet requirement
            if(value_buf_num >= VALUE_BUF_SIZE) {
                value_buf_num = VALUE_BUF_SIZE;

                int pred = predict(value_buf, VALUE_BUF_SIZE);

                // Output
                char output = pred + '0';
                UART_write(uart, &output, 1);
            }

            // Clear status
            idx = 0;
            memset(value_str, 0, VALUE_CHAR_SIZE);
            break;
        case '0': case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9': case '.':
            value_str[idx++] = input;
            break;
        }
        //UART_write(uart, &input, 1);
    }
}
