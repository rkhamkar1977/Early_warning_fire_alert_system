/*
 * main.c
 *
 *  Created on: 2016/10/14
 *      Author: User
 */

#include <stdio.h>
#include "system.h"
#include "esp8266.h"

int main()
{
    printf("Hello from Nios II!\n");

    esp8266_init(true);

    esp8266_listen();

    return 0;
}

