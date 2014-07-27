#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 512

int posy = 3;
int rflag = 0;
static SOCKET sock;

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

	//receive data.
	char buf[BUFSIZE + 1];
	int retval;

	while (1) {
		if (rflag == 1) return 0;
		retval = recv(sock, buf, BUFSIZE, 0);
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

	int retval;
	char buf[BUFSIZE + 1];
	
	while (1) {
		ZeroMemory(buf, BUFSIZE);
		gotoxy(1, 500);
		fgets(buf, BUFSIZE, stdin);

		if (strcmp(buf, ":exit\n") == 0) {
			retval = send(sock, buf, BUFSIZE, 0);
			rflag = 1;
			break;
		}
		retval = send(sock, buf, BUFSIZE, 0);
		if (retval == SOCKET_ERROR) err_display("send()");
	}

	return 0;
}

int main(int argc, char *argv[]) {
	system("title:Client");
	system("mode con:cols=70 lines=500");
	char ip_addr[16];
	//initialize winSock.
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) err_quit("socket()");

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	
	printf("Input Server Ip Address : >> ");
	fgets(ip_addr, 16, stdin);
	serveraddr.sin_addr.s_addr = inet_addr(ip_addr);

	if (connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR) {
		err_quit("connect()");
	}

	DWORD threadId1[2];

	HANDLE rcvThread = CreateThread(NULL, 0, receive_data, NULL, 0, &threadId1[0]);
	HANDLE sendThread = CreateThread(NULL, 0, send_data, NULL, 0, &threadId1[1]);


	while (1) {
		if (rflag == 1)
			break;
	}
	closesocket(sock);
	WSACleanup();
	return 0;
}