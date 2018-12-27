#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT   27015

char USERNAME[DEFAULT_PORT] = "user";
char PASSWORD[DEFAULT_PORT] = "pass";

int make_socket(SOCKET *sock) {

	*sock = socket(AF_INET, SOCK_STREAM, 0);
	if (*sock == -1) {
		printf("Could not create socket\n");
		return 2;
	}
	puts("Socket created\n");
	return 0;
}
int make_connection(SOCKET sock, struct sockaddr_in server) {
	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Connect failed.\n");
		return 1;
	}
	puts("Connected\n");
	return 0;
}
void make_server(struct sockaddr_in *server) {
	server->sin_addr.s_addr = INADDR_ANY;
	server->sin_family = AF_INET;
	server->sin_port = htons(DEFAULT_PORT);
}
void make_server2(struct sockaddr_in *server) {
	server->sin_addr.s_addr = inet_addr("192.168.217.1");
	server->sin_family = AF_INET;
	server->sin_port = htons(DEFAULT_PORT);
}
int make_bind(SOCKET sock, struct sockaddr_in server) {
	if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");
	return 0;
}
int recv_msg(SOCKET sock, char message[]) {
	int read_size;
	if (read_size = (recv(sock, message, DEFAULT_BUFLEN, 0)) > 0) {
		puts(message);
		return 0;
	}
	else if (read_size == 0) {
		puts("client disconnected");
		return 1;
	}
	else {
		puts("recv failed");
		return 1;
	}
}
int send_fun(SOCKET sock, const char message[DEFAULT_BUFLEN]) {
	if (send(sock, message, strlen(message), 0) < 0) {
		puts("Send failed");
		return 1;
	}
	return 0;
}

FILE *safe_fopen(char *filename, const char *mode, int error_code) {
	FILE *fp = fopen(filename, mode);
	if (fp == NULL) {
		printf("Can't open '%s'!\n", filename);
		exit(error_code);
	}
	return fp;
}

int main(int argc, char *argv[])
{
	// Initialize Winsock, request the Winsock 2.2
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		printf("Error at WSAStartup().\n");
		return 1;
	}
	else
	{
		printf("WSA library is succesfully initilized.\n");
	}

	SOCKET sock, socket_desc, client_sock;
	int c;
	struct sockaddr_in server, client;
	struct sockaddr_in server2;
	char User[DEFAULT_BUFLEN] = "\0";
	char Pass[DEFAULT_BUFLEN] = "\0";
	char message[DEFAULT_BUFLEN] = "\0";
	char message2[DEFAULT_BUFLEN] = "\0";
	//char check_message[DEFAULT_BUFLEN] = "check_message";
	//Create socket
	make_socket(&socket_desc);

	//Prepare the sockaddr_in structure
	make_server(&server);

	//Bind
	if (make_bind(socket_desc, server) == 1) {
		return 0;
	}
	//Listen
	listen(socket_desc, 3);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);

	//accept connection from an incoming client
	client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
	if (client_sock < 0)
	{
		perror("accept failed");
		return 1;
	}
	puts("Connection accepted");
	//Receive a message from client

	while (1) {
		recv_msg(client_sock, User);
		recv_msg(client_sock, Pass);
		if (strcmp(User, USERNAME) == 0 && strcmp(PASSWORD, Pass) == 0) {
			puts("konektovan");
			//send_fun(client_sock,"Connected");
			send_fun(client_sock, "1");
			break;
		}
		else {
			puts("konekcija nije uspela");
			send_fun(client_sock, "0");
		}
	}

	puts("Primanje podataka");

	//recv_msg(client_sock,message);

	char *out_filename = argv[1];

	FILE *out = safe_fopen(out_filename, "w", 1);

	int fr_block = 0;

	while (fr_block = recv(client_sock, message, DEFAULT_BUFLEN, 0)) {
		if (fr_block < 0) {
			perror("Error receiving file from client to server.\n");
		}
		int write_sz = fwrite(message, sizeof(char), fr_block, out);
		if (write_sz < fr_block) {
			perror("File write failed on server.\n");
		}
	}
	printf("OK received from proxy_server! \n");

	fclose(out);

	make_server2(&server2);
	make_socket(&sock);

	if (make_connection(sock, server2) == 1) {
		return 0;
	}
	//send_fun(sock,message);
	FILE *in = safe_fopen(out_filename, "r", 1);
	long int size = ftell(in); //uzima duzinu fajla
	int n = size / DEFAULT_BUFLEN;
	int i;
	int fs_block;
	for (i = 0; i < n + 1; i++)
	{
		while ((fs_block = fread(message2, sizeof(char), DEFAULT_BUFLEN, out)) > 0) {
			if (send(sock, message2, fs_block, 0) < 0) {
				printf("ERROR: Failed to send file %s. \n", out_filename);
				return 1;
			}
		}
	}
	printf("OK FILE %s from client was sent! \n", out_filename);

	remove(out_filename);
	fclose(in);
	closesocket(sock);
	WSACleanup();

	return 0;
}