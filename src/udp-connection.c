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

enum {
    VERBOSE_LEVEL_INFO = 0,
    VERBOSE_LEVEL_DEBUG,
    VERBOSE_LEVELS
};

enum {
    DEBUG_LEVEL_NONE = 0,
    DEBUG_LEVEL_INFO,
    DEBUG_LEVEL_DETAIL,
    DEBUG_LEVELS
};

/*
 * @brief - opens udp client
 */
int udp_client_open(udp_client_t *client)
{
    struct sockaddr_in *addr;

    if ((client->socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("DATA socket creation failed");
        exit(EXIT_FAILURE);
    }

    addr = &client->server_addr;
    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(client->udp_port);
    addr->sin_addr.s_addr = INADDR_ANY;

    return 0;
}

/**
 * @brief check for incoming data from socket, or to transmit outgoing data.
 * @param socket_fd -- socket descriptor for udp port
 * @return 1 if data is ready on the socket; 0 otherwise
 */
unsigned int check_socket(int socket_fd)
{
    /* prepare socket operation timeout */
    struct timeval socket_timeout;
    unsigned long long micros = 100000;
    socket_timeout.tv_sec = 0; /* number of seconds */
    socket_timeout.tv_usec = micros;

    /** get maximum socket fd and populate rset, wset for use by select() */
    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(socket_fd, &rset);
    int status = select(socket_fd + 1, &rset, NULL, NULL, &socket_timeout);
    if (status < 0) { return -1; }

    return (FD_ISSET(socket_fd, &rset)) ? 1 : 0;
}

#define RX_DATA_BUFF_LENGTH (4096)

static uint8_t rx_data_buff[RX_DATA_BUFF_LENGTH];
static unsigned int rx_data_head = 0;
static unsigned int rx_data_tail = 0;
static const unsigned int rx_data_mask = RX_DATA_BUFF_LENGTH - 1;

int drain_udp_socket(udp_client_t *client)
{
    int status = check_socket(client->socket_fd);
    if (status > 0) {
        static uint8_t tmp_rx_buff[512];
        socklen_t len = sizeof (client->server_addr);
        ssize_t n = recvfrom(client->socket_fd, tmp_rx_buff, sizeof (tmp_rx_buff), 0, (struct sockaddr *) &client->server_addr, &len);
        for (int i = 0; i < n; ++i) { /* unconditionally wrap rx buffer if not serviced. no checking for wraps */
            uint8_t byte = tmp_rx_buff[i];
            if (client->verbose_level > VERBOSE_LEVEL_INFO) { fprintf(stderr, "read udp packet: byte(%d) = %2.2x. head = %d\n", i, byte, rx_data_head); }
            rx_data_buff[rx_data_head] = byte;
            rx_data_head = (rx_data_head + 1) & rx_data_mask;
        }
    }
}

int udp_client_read(udp_client_t *client, void *buf, size_t n_bytes)
{
    uint8_t *data = (uint8_t *) buf;
    size_t index = 0;
    while (index < n_bytes) { /* TODO timeout operation */
        drain_udp_socket(client);
        while (rx_data_tail != rx_data_head) {
            uint8_t byte = rx_data_buff[rx_data_tail];
            data[index++] = byte;
            if (client->verbose_level > VERBOSE_LEVEL_INFO) {
                fprintf(stderr, "net-rx %ld/%ld-byte = %2.2x. head=%d, tail=%d\n", index, n_bytes, byte, rx_data_head, rx_data_tail);
            }
            rx_data_tail = (rx_data_tail + 1) & rx_data_mask;
            if (index == n_bytes) {
                break;
            }
        }
    }

    return index;
}

int udp_client_write(udp_client_t *client, void *buf, size_t n_bytes)
{
    ssize_t n;

    const uint8_t *pos = (const uint8_t *) buf;

    fprintf(stderr, "request to write %zd bytes\n", n_bytes);
    while (n_bytes) { /* TODO timeout operation */
        n = sendto(client->socket_fd, pos, n_bytes, 0, (const struct sockaddr *) &client->server_addr, sizeof(client->server_addr));
        if ((client->verbose_level > VERBOSE_LEVEL_INFO) && (n > 0)) {
            int max = n;
            if (client->debug_level == DEBUG_LEVEL_NONE) { max = (max <= 16) ? max : 16; } /* potentially a torrent of data only if debug */
            if (client->verbose_level > VERBOSE_LEVEL_INFO) {
                for (int i = 0; i < max; ++i) { fprintf(stderr, "net-tx %d/%ld-byte = %2.2x\n", i, n_bytes, pos[i]); }
            }
        }
        if (n < 1)  { return 1; }
        n_bytes -= n;
        pos += n;
    }
    return 0;
}

