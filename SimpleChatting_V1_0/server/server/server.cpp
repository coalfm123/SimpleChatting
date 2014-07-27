#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 512

typedef struct clientSocket {
	SOCKET listen_sock;
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
}clientSocket;

int posy = 3;
int rflag = 0;

void gotoxy(int x, int y) {
	COORD Pos = { x - 1, y - 1 };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

void err_quit(char *msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

void err_display(char *msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
	exit(-1);
}

DWORD WINAPI receive_data(LPVOID arg) {

	clientSocket *cSocket;
	cSocket = (clientSocket*)arg;
	//receive data.
	char buf[BUFSIZE + 1];
	int retval;

	while (1) {
		retval = recv(cSocket->client_sock, buf, BUFSIZE, 0);
		if (strcmp(buf, ":exit\n") == 0) {
			rflag = 1;
			return 0;
		}
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
		} else if (retval == 0) {
			return 0;
		}

		//print rcv data.
		buf[retval] = '\0';
		printf("[Receive Message] :: > %s", buf);
	}
	return 0;
}

DWORD WINAPI send_data(LPVOID arg) {
	clientSocket *cSocket;
	cSocket = (clientSocket*)arg;

	int retval;
	char buf[BUFSIZE + 1];

	while (1) {
		ZeroMemory(buf, BUFSIZE);
		gotoxy(1, 500);

		fgets(buf, BUFSIZE, stdin);

		if (rflag == 1) return 0;
		else retval = send(cSocket->client_sock, buf, BUFSIZE, 0);

		if (retval == SOCKET_ERROR) err_display("send()");
	}

	return 0;
}

int main(int argc, char *argv[]) {
	system("title:Server");
	system("mode con:cols=70 lines=500");

	int retval;
	int addrlen;

	//initialize winSock.
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	//socket.
	clientSocket cSocket;
	cSocket.listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (cSocket.listen_sock == INVALID_SOCKET) err_quit("socket()");



	//bind
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(cSocket.listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind");

	DWORD threadId1[2];


	//listen()
	retval = listen(cSocket.listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	addrlen = sizeof(cSocket.clientaddr);
	while (1) {
		gotoxy(1, 500);
		printf("Wating Client...");
		//accept()

		cSocket.client_sock = accept(cSocket.listen_sock, (SOCKADDR*)&cSocket.clientaddr, &addrlen);
		if (cSocket.client_sock == INVALID_SOCKET) {
			err_display("accept()");
		}
		gotoxy(1, 500);
		printf("\n[TCP SERVER] Client connection : IP Addr = %s, PORT No. : %d ",
			   inet_ntoa(cSocket.clientaddr.sin_addr), ntohs(cSocket.clientaddr.sin_port));

		rflag = 0;
		HANDLE rcvThread = CreateThread(NULL, 0, receive_data, (LPVOID)&cSocket, 0, &threadId1[0]);
		HANDLE sendThread = CreateThread(NULL, 0, send_data, (LPVOID)&cSocket, 0, &threadId1[1]);

		while (1) {
			if (rflag == 1) break;
		}

		closesocket(cSocket.client_sock);
		printf("[TCP SERVER] Close Client : IP ADDR = %s, PORT No. = %d\n",
			   inet_ntoa(cSocket.clientaddr.sin_addr), ntohs(cSocket.clientaddr.sin_port));
	}
	//closesocket()
	closesocket(cSocket.listen_sock);

	//exit winsock
	WSACleanup();

	return 0;
}