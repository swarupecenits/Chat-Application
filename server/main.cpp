#include<iostream>
#include<tchar.h>
#include<WS2tcpip.h>
#include<WinSock2.h>
#include<thread>
#include<vector>

#define PORT 8080

using namespace std;

#pragma comment(lib,"ws2_32.lib")

/*
  Steps to create the chat app:

  initialize winsock lib

  create the socket

  get the ip and port on which it will run

  bind the ip and port with the socket

  listen on the socket

  accept

  receive and send

  close the socket

  cleanup the winsock
*/


bool Initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void InteractWithClient(SOCKET clientSocket ,vector<SOCKET>&clients) {
    //send and recv with client
    cout << "Client Connected" << endl;

    char buffer[4096];
    while (1)
    {
        int bytesrecvd = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesrecvd <= 0) {

            cout << "Client Disconnected" << endl;
            break;
        }

        string message(buffer, bytesrecvd);
        cout << "Message from Client:" << message << endl;

        for (auto client : clients) {
            if(client!=clientSocket){
                send(client, message.c_str(), message.length(), 0);
            }
            
        }
    }

    auto it = find(clients.begin(), clients.end(), clientSocket);
    if (it != clients.end()) {
        clients.erase(it);
    }

    closesocket(clientSocket);

}


int main() {
    if (!Initialize()) {
        cout << "Winsock Initialization Failed"<<endl;
        return 1;
    }

    cout << "server program" << endl;

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM,0);//using ipv4 and tcpip protocol and 0 indicate the service provider decides the protocol automatically

    if (listenSocket == INVALID_SOCKET) {
        cout << "Socket Creation Failed" << endl;
        return 1;
    }

    //create address structure
    int port = PORT;
    sockaddr_in serveraddr;
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port = htons(port);//host to network


    //convert the ipaddress (0.0.0.0) put it inside the sin_family in binary format
    if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1) {
        cout << "Setting Address Structure Failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    //bind
    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
        cout << "Binding Failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    //Listen on the Port
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Listening failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }//somaxconn is the max connection it can handle


    cout << "Server Has Started Listening on PORT:"<<port << endl;
    vector<SOCKET> clients;

    while (1) {
        //Accept connections from the client
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            cout << "Client Socket Failed" << endl;
        }

        clients.push_back(clientSocket);//to save the clients connected to the server

        thread t1(InteractWithClient, clientSocket,std::ref(clients));
        t1.detach();
    }
    
    closesocket(listenSocket);
   

    WSACleanup();
    return 0;
}