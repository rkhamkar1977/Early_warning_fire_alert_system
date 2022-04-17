/*
 * esp8266.c
 *
 *  Created on: 2016/10/7
 *      Author: User
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "system.h"
#include <fcntl.h>
#include <unistd.h>
#include "esp8266.h"
#include <string.h>
#include "web.h"
#include <altera_avalon_pio_regs.h>

#define INDEX_LEN 510
#define FIRE_LEN 480
#define esp8266_uart WIFI_UART0_NAME
//#define ESP8266_DEBUG

FILE *esp8266_file;
char buffer[1024];
unsigned char * http_resp = index_html;
int http_resp_len = INDEX_LEN;

struct fire_history {
	int node;
	bool react;
	int sense_data[2];
};

struct http_request {
    enum http_methods_enum {
        GET, POST, OTHER
    } http_methods;
    char *path;
    bool connected;
    int id;
};

struct alert_data {
	int node;
	float sense_data[3];
};


void set_esp8266_file_blocking(bool blocking)
{
    if (blocking == false) {
        fcntl(fileno(esp8266_file), F_SETFL, O_NONBLOCK);
    } else {
        int file_fl = fcntl(fileno(esp8266_file), F_GETFL);
        file_fl = file_fl & ~O_NONBLOCK;
        fcntl(fileno(esp8266_file), F_SETFL, file_fl);
    }
}

bool esp8266_init(bool reset)
{
    bool bSuccess = true;
    esp8266_file = fopen(esp8266_uart, "rw+");
    if (esp8266_file == NULL) {
        printf("Open UART_0 failed\n");
        return false;
    }

    if (reset) {
        IOWR_ALTERA_AVALON_PIO_DATA(PIO_WIFI_RESET_BASE, 0);
        usleep(50);
        IOWR_ALTERA_AVALON_PIO_DATA(PIO_WIFI_RESET_BASE, 1);
        usleep(3 * 1000 * 1000);
        esp8266_dump_rx();
    }

    esp8266_send_command("AT+CWSAP_CUR=\"Terasic_RFS\",\"1234567890\",5,3");
    esp8266_send_command("AT+CWMODE_CUR=2");
    esp8266_send_command("AT+CWLIF");

    return bSuccess;
}

char *get_line_noblock()
{
    set_esp8266_file_blocking(false);
    buffer[0] = fgetc(esp8266_file);
    set_esp8266_file_blocking(true);
    if (buffer[0] != EOF) {
        if (fgets(buffer + 1, sizeof(buffer) - 1, esp8266_file) != NULL)
            return buffer;
    }
    return NULL;
}

bool esp8266_send_command(const char *cmd)
{
    fprintf(esp8266_file, "%s\r\n", cmd);
    int length = 0;
    while (1) {
        if (fgets(buffer + length, sizeof(buffer) - length,
                esp8266_file) != NULL) {
#ifdef ESP8266_DEBUG
            printf("%s", buffer + length);
#endif
            if (strstr(buffer + length, "OK") != NULL) {
#ifndef ESP8266_DEBUG
                if (strcmp("AT+CWLAP", cmd) == 0) {
                    printf("%s", buffer);
                }
#endif
                return true;
            } else if (strstr(buffer + length, "ERROR") != NULL) {
                return false;
            } else if (strstr(buffer + length, "FAIL") != NULL) {
                return false;
            }
            length += strlen(buffer + length);
        }
    }
    return false;
}

bool esp8266_send_data(const char *data, int length)
{
    write(fileno(esp8266_file), data, length);

    length = 0;
    while (1) {
        if (fgets(buffer + length, sizeof(buffer) - length,
                esp8266_file) != NULL) {
#ifdef ESP8266_DEBUG
            printf("%s", buffer + length);
#endif
            if (strstr(buffer + length, "SEND OK") != NULL) {
                return true;
            } else if (strstr(buffer + length, "SEND FAIL") != NULL) {
#ifndef ESP8266_DEBUG
                printf("%s", buffer);
#endif
                return false;
            }
            length += strlen(buffer + length);
        }
    }
    return false;
}

void esp8266_dump_rx()
{
    set_esp8266_file_blocking(false);
    while (fgets(buffer, sizeof(buffer), esp8266_file) != NULL) {
#ifdef ESP8266_DEBUG
        printf("%s", buffer);
#endif
    }
    set_esp8266_file_blocking(true);
    fflush(stdout);
}

bool http_request_parse(const char *str, struct http_request *request,
        int *o_id)
{
    struct http_request request_tmp;
    bool success = true;
    int id;
    int length;
    char methods[10];
    char *first_space = strchr(str, ' ');
    int path_length;
    if ((path_length = strchr(first_space + 1, ' ') - first_space) <= 0) {
        success = false;
    }
    char *path;
    if (success) {
        path = (char *) malloc(sizeof(char) * path_length);
        if (sscanf(str, "+IPD,%d,%d:%s %s ", &id, &length, methods, path)
                != 4) {
            success = false;
        }
    }
    if (success) {
        if (strcmp(methods, "GET") == 0) {
            request_tmp.http_methods = GET;
        } else if (strcmp(methods, "POST") == 0) {
            request_tmp.http_methods = POST;
        } else {
            request_tmp.http_methods = POST;
        }
    }

    if (success) {
        free(request[id].path);
        request[id].path = path;
        request[id].http_methods = request_tmp.http_methods;
        *o_id = id;
    } else {
        free(path);
    }
    return success;
}

bool http_response_send(struct http_request *request, const char *head,
        const char *body, int body_length)
{
    bool success = true;
    char cmd_buffer[100];
    char data_buffer[1024];
    if (body_length == 0)
        body_length = strlen(body);
    int bytes_left = body_length;
    int chunk_bytes;
    char *body_p = (char *) body;

    sprintf(data_buffer, head, body_length);
    sprintf(cmd_buffer, "AT+CIPSEND=%d,%d", request->id, strlen(data_buffer));
    if (success)
        success = esp8266_send_command(cmd_buffer);
    if (success)
        success = esp8266_send_data(data_buffer, strlen(data_buffer));

    while (bytes_left > 0 && success) {
        if (bytes_left >= 1000)
            chunk_bytes = 1000;
        else
            chunk_bytes = bytes_left;

        sprintf(cmd_buffer, "AT+CIPSEND=%d,%d", request->id, chunk_bytes);
        if (success)
            success = esp8266_send_command(cmd_buffer);
        if (success)
            success = esp8266_send_data(body_p, chunk_bytes);

        body_p += chunk_bytes;
        bytes_left -= chunk_bytes;
    }

    sprintf(cmd_buffer, "AT+CIPCLOSE=%d", request->id);
    if (success)
        success = esp8266_send_command(cmd_buffer);
    request->connected = false;

    return success;
}

bool led_response(struct http_request *request)
{
    bool success = true;
    int led_num;
    int led;
    if (strstr(request->path, "/led/on") == request->path) {
        if (sscanf(request->path, "/led/on/%d", &led_num) == 1) {
            led = IORD_ALTERA_AVALON_PIO_DATA(PIO_LED_BASE);
            led = led | (1 << led_num);
            IOWR_ALTERA_AVALON_PIO_DATA(PIO_LED_BASE, led);
        }
    } else if (strstr(request->path, "/led/off") == request->path) {
        if (sscanf(request->path, "/led/off/%d", &led_num) == 1) {
            led = IORD_ALTERA_AVALON_PIO_DATA(PIO_LED_BASE);
            led = led & ~(1 << led_num);
            IOWR_ALTERA_AVALON_PIO_DATA(PIO_LED_BASE, led);
        }
    }
    http_response_send(request, __302_head, (char *) __302_html,
            __302_html_len);
    return success;
}

struct alert_data fire_response(struct http_request *request)
{
    struct alert_data incoming_data;
    int node;
    int led;
    float sense_data[3];
    sscanf(request->path, "/SRDURDKFIR?%d?%f?%f?%f", &node, &sense_data[0], &sense_data[1], &sense_data[2]);
    incoming_data.node = node;
    incoming_data.sense_data[0] = sense_data[0];
    incoming_data.sense_data[1] = sense_data[1];
    incoming_data.sense_data[2] = sense_data[2];
    http_resp = fire_html;
    http_resp_len = fire_html_len;
    led = IORD_ALTERA_AVALON_PIO_DATA(PIO_LED_BASE);
    led = led | (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0);
    IOWR_ALTERA_AVALON_PIO_DATA(PIO_LED_BASE, led);
    http_response_send(request, __302_head, (char *) __302_html,
            __302_html_len);
    return incoming_data;
}

bool fire_react(struct http_request *request, struct alert_data *node_data, int *count, struct fire_history *hist)
{
    bool success = true;
    int led;
    if (strstr(request->path, "/fire_resp/true") == request->path) {
        http_resp = index_html;
        http_resp_len = index_html_len;
        led = IORD_ALTERA_AVALON_PIO_DATA(PIO_LED_BASE);
        led = led & ~(1 << 3);
        led = led & ~(1 << 1);
        led = led & ~(1 << 2);
        led = led & ~(1 << 0);
        IOWR_ALTERA_AVALON_PIO_DATA(PIO_LED_BASE, led);
        hist[*count].node = node_data->node;
        hist[*count].react = true;
        hist[*count].sense_data[0] = node_data->sense_data[0];
        hist[*count].sense_data[1] = node_data->sense_data[1];
        hist[*count].sense_data[2] = node_data->sense_data[2];
        (*count)++;

    } else if (strstr(request->path, "/fire_resp/false") == request->path) {
        http_resp = index_html;
        http_resp_len = index_html_len;
        led = IORD_ALTERA_AVALON_PIO_DATA(PIO_LED_BASE);
        led = led & ~(1 << 3);
        led = led & ~(1 << 1);
        led = led & ~(1 << 2);
        led = led & ~(1 << 0);
        IOWR_ALTERA_AVALON_PIO_DATA(PIO_LED_BASE, led);
        hist[*count].node = node_data->node;
        hist[*count].react = false;
        hist[*count].sense_data[0] = node_data->sense_data[0];
        hist[*count].sense_data[1] = node_data->sense_data[1];
        hist[*count].sense_data[2] = node_data->sense_data[2];
        (*count)++;
    }
    return success;
}


void esp8266_listen()
{
    int id, length = 0;
    const int port = 80;
    struct fire_history hist[100];
    int count = 0;
    char buffer[1000];
    char table[1000];
    strcpy(table,"");
	strcat(table, "<table>\n");
	strcat(table, "<tr>\n<th>Node</th>\n<th>True?</th>\n<th>Sensor data</th>\n<th>Sensor data</th>\n<th>Sensor data</th>\n</tr>\n");
    esp8266_send_command("AT+CIPMUX=1");
    esp8266_send_command("AT+CIPSERVER=1,80");
    printf("Server has started to listen at port: %d.", port);
    printf("\n%s\n", table);
    struct http_request request[4];
    for (int i = 0; i < 4; i++) {
        request[i].path = NULL;
        request[i].connected = false;
        request[i].id = i;
    }
    struct alert_data node_data;
    while (1) {
        while (1) {

            if (fgets(buffer, sizeof(buffer), esp8266_file) == NULL) {
                continue;
            }
            printf("%s", buffer);
            if (strstr(buffer, "+IPD") != NULL) {
                length = strlen(buffer);
                while (fgets(buffer + length, sizeof(buffer) - length,
                        esp8266_file) != NULL) {
                    if (strcmp(buffer + length, "\r\n") == 0)
                        break;
                    length += strlen(buffer + length);
                }
                break;
            } else if (sscanf(buffer, "%d,CONNECT", &id) == 1) {
                request[id].connected = true;
            } else if (sscanf(buffer, "%d,CLOSED", &id) == 1) {
                request[id].connected = false;
            }
        }

        if (http_request_parse(buffer, request, &id)) {
            if (request[id].connected == true) {
                if (strcmp(request[id].path, "/") == 0) {
                    http_response_send(&request[id], html_head,
                            (char *) http_resp, http_resp_len);
                } else if (strstr(request[id].path, "/SRDURDKFIR?") == request[id].path) {
                	node_data = fire_response(&request[id]);
                	http_response_send(&request[id], html_head,
                	        (char *) fire_html, fire_html_len);
                }
                else if (strstr(request[id].path, "/fire_resp") == request[id].path) {
                    fire_react(&request[id], &node_data, &count, hist);
            		sprintf(buffer,"<tr>\n<td>%d</td>\n<td>%d</td>\n<td>%d</td>\n<td>%d</td>\n</tr>\n",hist[count-1].node,hist[count-1].react,hist[count-1].sense_data[0],hist[count-1].sense_data[1]);
            		strcat(table,buffer);
                    http_response_send(&request[id], html_head,
                                                (char *) http_resp, http_resp_len);
                }
                else if (strcmp(request[id].path, "/favicon.ico") == 0) {
                    http_response_send(&request[id], favicon_ico_head,
                            (char *) favicon_ico, favicon_ico_len);
                }
                else if (strcmp(request[id].path, "/fire_data.html") == 0) {
                    strcpy(buffer,table);
                    strcat(buffer,"</table>\0");
                	http_response_send(&request[id], html_head,
                    		(char *) buffer, strlen(buffer));
                }
                else if (strstr(request[id].path, "/led")
                        == request[id].path) {
                    led_response(&request[id]);
                } else {
                    http_response_send(&request[id], __404_head,
                            (char *) __404_html, __404_html_len);
                }
            }
        }
    }
}
