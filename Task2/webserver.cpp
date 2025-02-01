// webserver.cpp
#include <iostream>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <queue>
#include <signal.h>
#include <cstring>

#define PORT 8080
#define THREAD_POOL_SIZE 10
#define MAX_CONNECTIONS 100

struct ThreadPool {
    std::queue<int> tasks;
    pthread_t* threads;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    bool shutdown;

    ThreadPool(size_t num_threads) : shutdown(false) {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&condition, NULL);
        threads = new pthread_t[num_threads];
    }

    ~ThreadPool() {
        delete[] threads;
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&condition);
    }
};

ThreadPool* pool = nullptr;

void send_http_response(int client_socket) {
    const char* response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>Hello from Multithreaded Server!</h1></body></html>";
    
    write(client_socket, response, strlen(response));
}

void* handle_connection(void* arg) {
    char buffer[1024];

    while (true) {
        int client_socket;
        
        pthread_mutex_lock(&pool->mutex);
        while (pool->tasks.empty() && !pool->shutdown) {
            pthread_cond_wait(&pool->condition, &pool->mutex);
        }
        
        if (pool->shutdown && pool->tasks.empty()) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        
        client_socket = pool->tasks.front();
        pool->tasks.pop();
        pthread_mutex_unlock(&pool->mutex);

        // Handle client request
        ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::cout << "Received request from client\n";
            send_http_response(client_socket);
        }

        close(client_socket);
    }
    return nullptr;
}

void signal_handler(int sig) {
    if (pool) {
        pthread_mutex_lock(&pool->mutex);
        pool->shutdown = true;
        pthread_cond_broadcast(&pool->condition);
        pthread_mutex_unlock(&pool->mutex);
    }
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "setsockopt failed\n";
        return 1;
    }

    // Configure server address
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);


    // Bind socket
    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }

    // Listen for connections
    if (listen(server_socket, MAX_CONNECTIONS) < 0) {
        std::cerr << "Listen failed\n";
        return 1;
    }

    // Initialize thread pool
    pool = new ThreadPool(THREAD_POOL_SIZE);
    
    // Set up signal handler
    signal(SIGINT, signal_handler);

    // Create worker threads
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        if (pthread_create(&pool->threads[i], NULL, handle_connection, NULL) != 0) {
            std::cerr << "Failed to create thread\n";
            return 1;
        }
    }

    std::cout << "Server started on port " << PORT << std::endl;

    // Accept connections
    while (!pool->shutdown) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) continue;

        pthread_mutex_lock(&pool->mutex);
        pool->tasks.push(client_socket);
        pthread_cond_signal(&pool->condition);
        pthread_mutex_unlock(&pool->mutex);
    }

    // Wait for threads to finish
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    close(server_socket);
    delete pool;
    return 0;
}