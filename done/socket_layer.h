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

/**
 * @brief Reads data from a TCP socket into a buffer.
 *
 * This function reads up to `buflen` bytes of data from the specified `active_socket`
 * and stores it in the provided `buf` buffer.
 *
 * @param active_socket The active TCP socket to read from.
 * @param buf The buffer to store the read data.
 * @param buflen The maximum number of bytes to read.
 *
 * @return The number of bytes read on success, or a negative error code on failure.
 *         Possible error codes include ERR_INVALID_ARGUMENT if the `active_socket` is invalid
 *         or `buflen` is less than 0.
 */
ssize_t tcp_read(int active_socket, char* buf, size_t buflen);

/**
 * @brief Sends a TCP message over the network.
 *
 * @param active_socket The active socket to send the message on.
 * @param response The message to send.
 * @param response_len The length of the message.
 * @return The number of bytes sent on success, or an error code on failure.
 */
ssize_t tcp_send(int active_socket, const char* response, size_t response_len);
