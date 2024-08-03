#include<iostream>
#include<tchar.h>
#include<WS2tcpip.h>
#include<WinSock2.h>
#include<thread>
#include<string>
#include<vector>
#include <ctime> 

#define PORT 8080

using namespace std;

#pragma comment(lib,"ws2_32.lib")

/*
	Initialize winsock

	Create socket

	Connect to the server

	Send/recv

	Close the socket
*/

// Color codes
#define RED "\033[1;31m"
#define RESET "\033[0m"
#define GREEN "\033[0;32m"
#define BLUE "\033[0;34m"
#define YELLOW "\033[0;33m"
#define PURPLE "\033[0;35m"
#define CYAN "\033[0;36m"

vector<string> colors = { RED,GREEN,YELLOW,BLUE,PURPLE ,CYAN };


// Function to enable ANSI escape codes in Windows console
void enableVirtualTerminalProcessing() {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE) return;

	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode)) return;

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, dwMode);
}

bool Initialize() {
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void SendMsg(SOCKET s) {
	cout << "Enter your username:" << endl;
	string name;
	getline(cin, name);
	string message;

	while (true)
	{
		getline(cin, message);
		
		// Generate a random index for the color
		int colorIndex = rand() % colors.size();
		string randomColor = colors[colorIndex];

		// Color the username
		string coloredName = randomColor + name + RESET;

		// Construct the message
		string msg = coloredName + " : " + message;

		int bytessend=send(s, msg.c_str(), msg.length(),0);
		if (bytessend == SOCKET_ERROR) {
			cout << "Message Sending Failed!" << endl;
			break;
		}

		if (message == "quit") {
			cout << "User Disconnected from Server Successfully" << endl;
			cout << "Stopping the Application" << endl;
			break;
		}
	}
	closesocket(s);
	WSACleanup();
}

void ReceiveMsg(SOCKET s) {

	char buffer[4096];
	int recv_length;
	string msg="";


	while (true)
	{
		recv_length=recv(s, buffer, sizeof(buffer),0);
		if (recv_length <= 0) {
			cout << "Disconnected from Server" << endl;
			break;
		}
		else {
			msg = string(buffer, recv_length);
			cout << msg << endl;
		}
	}
	closesocket(s);
	WSACleanup();
}

int main() {

	// Enable ANSI escape codes
	enableVirtualTerminalProcessing();

	// Initialize random seed
	srand(static_cast<unsigned int>(time(0)) ^ _getpid());

	//Initialize
	if (!Initialize()) {
		cout << "Winsock Initialization Failed" << endl;
		return 1;
	}

	cout << "Client program" << endl;

	//Socket Creation
	SOCKET s;
	s= socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		cout << "Socket Creation Failed" << endl;
		return 1;
	}

	int port = PORT;
	string serveraddress = "127.0.0.1";//Local Server Code

	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	inet_pton(AF_INET, serveraddress.c_str(), &(serveraddr.sin_addr));

	if (connect(s, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
		cout << "Unable to connect to the Server" << endl;
		closesocket(s);
		WSACleanup();
		return 1;
	}

	cout << "Succesfully Connected to the Server" << endl;

	thread senderThread(SendMsg, s);
	thread receiverThread(ReceiveMsg, s);

	senderThread.join();
	receiverThread.join();

	return 0;
}