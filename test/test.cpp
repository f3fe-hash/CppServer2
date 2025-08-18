#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>

#define BUFFER_SIZE 8192

std::string SERVER_IP      = "192.168.1.39";    // Server IP to test
std::string SERVER_PAGE    = "/";               // Server page to test
constexpr int SERVER_PORT  = 8080;              // Server port to test
constexpr int NUM_REQUESTS = 30000;             // Number of requests to test the server with

double time_diff(struct timeval start, struct timeval end)
{
    return (end.tv_sec - start.tv_sec) * 1e6 + 
       (end.tv_usec - start.tv_usec);
}

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    double total_time = 0.00;

    // Hide console cursor
    printf("\033[?25l"); 

    // --- Build HTTP Request ---
    char req[512];
    snprintf(req, 512,
             "GET %s HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "User-Agent: stress-client\r\n"
             "Connection: close\r\n\r\n",
             SERVER_PAGE.c_str(), SERVER_IP.c_str(), SERVER_PORT);
    
    // Cast to const char* to save performance
    const char* request = (const char *)req;

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &server.sin_addr);

    struct timeval total_start, total_end;
    gettimeofday(&total_start, NULL);

    for (int i = 0; i < NUM_REQUESTS; i++)
    {
        // Setup Socket
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            perror("Socket creation failed");
            continue;
        }

        // Connect
        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            close(sock);
            continue;
        }

        // Send Request
        struct timeval send_start, send_end;
        gettimeofday(&send_start, NULL);

        ssize_t sent = send(sock, request, strlen(request), 0);
        if (sent < 0)
        {
            perror("Send failed");
            close(sock);
            continue;
        }

        gettimeofday(&send_end, NULL);
        double send_time = time_diff(send_start, send_end);

        struct timeval recv_start, recv_end;
        gettimeofday(&recv_start, NULL);

        // Receive Response
        char buffer[BUFFER_SIZE];
        while (recv(sock, buffer, BUFFER_SIZE - 1, 0) > 0);

        gettimeofday(&recv_end, NULL);
        double recv_time = time_diff(recv_start, recv_end);

        close(sock);

        double request_total = send_time + recv_time;
        total_time += request_total;

        printf("[Request %d (%.2f%%, %d req / ms)]\r",
            i + 1,
            (double)((double)(i + 1) / (double)NUM_REQUESTS) * 100,
            (int)(1000 / request_total));

    }

    gettimeofday(&total_end, NULL);

    // Show console cursor
    printf("\033[?25h\n");

    double total_elapsed_us = time_diff(total_start, total_end);
    double total_elapsed_sec = total_elapsed_us / 1e6;
    double req_per_sec = NUM_REQUESTS / total_elapsed_sec;

    printf("=== Total wall time for %d requests: %.2f ms ===\n", NUM_REQUESTS, total_elapsed_us / 1000);
    printf("=== Average latency per request: %.2f µs ===\n", total_time / NUM_REQUESTS);
    printf("=== Overall throughput: %.2f requests / sec ===\n", req_per_sec);

    return 0;
}