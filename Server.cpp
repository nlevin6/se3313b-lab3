#include "socketserver.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <vector>
#include <atomic>

using namespace Sync;

std::atomic<bool> terminateServer(false);// Global atomic flag to control server termination
std::vector<std::thread> clientThreads;// Vector to store client threads

void HandleClient(Socket client) {
    try {
        // As logn as server is running, read data
        while (!terminateServer) {
            ByteArray data;
            int bytesRead = client.Read(data);

            // Message when client leaves server
            if (bytesRead <= 0) {
                std::cout << "Client disconnected" << std::endl;
                break;
            }

            // Convert received message to upper case
            std::transform(data.v.begin(), data.v.end(), data.v.begin(), ::toupper);

            // Send back the transformed data
            std::cout << "Transformed message: " << data.ToString() << std::endl;
            client.Write(data);
        }
    } catch (const std::string &error) {
        std::cerr << "Error: " << error << std::endl;
    }
}

// Continuously read input from server terminal
void ReadServerInput(SocketServer& server) {
    std::string input;
    while (true) {
        std::getline(std::cin, input);
        // If user wants to stop the server to gracefully terminate it
        if (input == "stop server") {
            std::cout << "Received request to stop server. Terminating..." << std::endl;
            terminateServer = true;
            server.Shutdown();
            break;
        }
    }
}

int main() {
    try {
        SocketServer server(3002);
        std::cout << "I am a server" << std::endl;

        // Start a thread to continuously read input from server terminal
        std::thread inputThread(ReadServerInput, std::ref(server));

        while (!terminateServer) {
            Socket client = server.Accept();
            std::cout << "Client connected" << std::endl;

            // Create a thread to handle the client connection
            std::thread clientThread(HandleClient, std::move(client));
            clientThreads.push_back(std::move(clientThread));
        }

        // Wait for all client threads to finish
        for (auto& thread : clientThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        // Wait for the input thread to finish
        inputThread.join();
    } catch (const std::string &error) {
        std::cerr << "Error: " << error << std::endl;
        return 1;
    }
    std::cout << "Server terminated gracefully." << std::endl;
    return 0;
}
