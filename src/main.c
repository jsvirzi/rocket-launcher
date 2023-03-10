#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
#include <pthread.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include "udp-connection.h"

// #include <curl/curl.h>

int main() {
    udp_client_t udp_client_data[1];
    udp_client_t udp_client_cmmd[1];

    memset(udp_client_data, 0, sizeof (udp_client_t));
    memset(udp_client_cmmd, 0, sizeof (udp_client_t));
    
    udp_client_data->udp_port = 55151;
    udp_client_cmmd->udp_port = udp_client_data->udp_port + 1;
    udp_client_open(udp_client_data);
    udp_client_open(udp_client_cmmd);
    // curl_global_init(CURL_GLOBAL_DEFAULT);

    // udp_client_data->verbose_level = 1;
    // udp_client_cmmd->verbose_level = 1;
    // udp_client_data->debug_level = 1;
    // udp_client_cmmd->debug_level = 1;

    uint8_t command_string[64];
    ssize_t command_length;

    command_length = snprintf((char *) command_string, sizeof (command_string), "$DTR,0*");
    udp_client_write(udp_client_cmmd, command_string, command_length);

    command_length = snprintf((char *) command_string, sizeof (command_string), "$RTS,1*");
    udp_client_write(udp_client_cmmd, command_string, command_length);

    command_length = snprintf((char *) command_string, sizeof (command_string), "$RTS,0*");
    udp_client_write(udp_client_cmmd, command_string, command_length);

    /* write something for server to capture address */
    udp_client_write(udp_client_data, command_string, command_length);

    drain_udp_socket(udp_client_data);

    while (1) {
        char data_string[64];
        ssize_t data_length;
        data_length = udp_client_read(udp_client_data, data_string, sizeof (data_string));
        printf("%zd bytes read\n", data_length);
        for (int i = 0; i < data_length; ++i) {
            uint8_t byte = data_string[i];
            if (byte == '$') { printf("\n"); }
            if (byte == '*') { printf("\n"); }
            printf("%c", data_string[i]);
        }
    }

    return 0;
}
