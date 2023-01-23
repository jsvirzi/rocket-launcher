#ifndef UDP_CONNECTION_H
#define UDP_CONNECTION_H

#include <inttypes.h>
#include <pthread.h>

typedef struct _UdpClient
{
    int verbose_level;
    int debug_level;
    uint32_t sunset_expiry;
    unsigned int sunset_period;
    struct timespec ts0;
    unsigned long long ts0_secs;
    int run_change_request;
    pthread_t tid;
    int udp_port;
    int socket_fd;
    struct sockaddr_in server_addr;
    // struct sockaddr_in client_addr;
    unsigned int socket_timeout;
    unsigned int socket_period;
    unsigned int queue_overflow_policy;
    uint32_t dropped_rx_packets;
    uint32_t dropped_tx_packets;
    uint32_t n_rx;
    uint32_t n_tx;
    uint32_t n_inconsistent_size_errors;
    unsigned int connection_state;
    uint32_t connection_state_timeout;
    unsigned int status_report_period;
    uint32_t status_report_expiry;
    unsigned int keep_alive_period;
    uint32_t keep_alive_expiry;
    unsigned char rset_flag;
    unsigned char wset_flag;
    int echo_flag;
    int tx_dir_active;
    int run;
} UdpClient;

typedef UdpClient udp_client_t;

int udp_client_open(udp_client_t *client);
int udp_client_write(udp_client_t *client, void *buf, size_t n_bytes);
int udp_client_read(udp_client_t *client, void *buf, size_t n_bytes);
int drain_udp_socket(udp_client_t *client);
unsigned int check_socket(int socket_fd);

#endif

