/**
 * @file socket_layer.h
 * @brief Minimal socket layer.
 *
 * @author Konstantinos Prasopoulos
 */

#pragma once

#include <stddef.h> // size_t
#include <stdint.h> // uint16_t
#include <sys/types.h> // ssize_t

int tcp_server_init(uint16_t port);

/**
 * @brief Blocking call that accepts a new TCP connection
 */
int tcp_accept(int passive_socket);

/**
 * @brief Blocking call that reads the active socket once and stores the output in buf
 */
ssize_t tcp_read(int active_socket, char* buf, size_t buflen);

ssize_t tcp_send(int active_socket, const char* response, size_t response_len);
