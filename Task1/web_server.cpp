// Multithreaded Web Server
#include <iostream>
#include <pthread.h>
#include <queue>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <atomic>
#include <signal.h>

const int MAX_THREADS = 10;
const int PORT = 8080;

struct ThreadPool {
    std::queue<int> clientQueue;
    pthread_t threads[MAX_THREADS];
    pthread_mutex_t queueMutex;
    pthread_cond_t queueCond;
    std::atomic<bool> shutdownFlag{false};
    
    ThreadPool() {
        pthread_mutex_init(&queueMutex, NULL);
        pthread_cond_init(&queueCond, NULL);
    }
    
    ~ThreadPool() {
        pthread_mutex_destroy(&queueMutex);
        pthread_cond_destroy(&queueCond);
    }
};

ThreadPool* pool;

void* handleClient(void* arg) {
    char buffer[1024];
    
    while (!pool->shutdownFlag) {
        int clientSocket;
        
        pthread_mutex_lock(&pool->queueMutex);
        while (pool->clientQueue.empty() && !pool->shutdownFlag) {
            pthread_cond_wait(&pool->queueCond, &pool->queueMutex);
        }
        
        if (pool->shutdownFlag) {
            pthread_mutex_unlock(&pool->queueMutex);
            break;
        }
        
        clientSocket = pool->clientQueue.front();
        pool->clientQueue.pop();
        pthread_mutex_unlock(&pool->queueMutex);
        
        // Handle client request
        read(clientSocket, buffer, sizeof(buffer));
        
        // Simple HTTP response
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
        write(clientSocket, response.c_str(), response.length());
        
        close(clientSocket);
    }
    return NULL;
}

void signalHandler(int sig) {
    if (pool) {
        pool->shutdownFlag = true;
        pthread_cond_broadcast(&pool->queueCond);
    }
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    bind(serverSocket, (struct sockaddr *)&address, sizeof(address));
    listen(serverSocket, 5);
    
    pool = new ThreadPool();
    signal(SIGINT, signalHandler);
    
    // Create thread pool
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_create(&pool->threads[i], NULL, handleClient, NULL);
    }
    
    while (!pool->shutdownFlag) {
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket < 0) continue;
        
        pthread_mutex_lock(&pool->queueMutex);
        pool->clientQueue.push(clientSocket);
        pthread_cond_signal(&pool->queueCond);
        pthread_mutex_unlock(&pool->queueMutex);
    }
    
    // Cleanup
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    close(serverSocket);
    delete pool;
    return 0;
}