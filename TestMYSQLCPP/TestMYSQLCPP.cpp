// TestMYSQLCPP.cpp : Defines the entry point for the console application.
//
#pragma comment(lib,"libmysql.lib") 
#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")
//using namespace std;
//#include<iostream>
//#include<string>
//#include<list>
#include<stdio.h>
#include<conio.h>
#include <stdio.h>
#include <winsock2.h> // Wincosk2.h ������ ���� ������ windows!
#include <windows.h>
#include <mysql.h>
#define MY_PORT 80 // ����, ������� ������� ������
#pragma warning(disable:4996)


int number = 0;
bool sender = false;
int senderlock = 0;
int locklist[3];//������ ��������, ������� ��� �������� ������

//������� ������ ������� �� ������� ��������
void ServerAnswer(SOCKET  my_sock, char *value)
{
	char lenght[1024];
	char msg[1024];
	char answer[1024];
	strcpy(answer,value);
	itoa(sizeof answer, lenght, 10);
	strcpy(msg, "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ");
	strcat(msg, lenght);
	strcat(msg, "\r\n");
	strcat(msg, "Content:");
	strcat(msg, answer);
	strcat(msg, "\r\n\r\n");
	send(my_sock, msg, sizeof msg, 0);
	printf("\n\n%s\n\n",msg);
}


//������� ���������� ����� ������ � ��
void WriteData(SOCKET my_sock, char *request)
{
	MYSQL conn;
	char query[100];
	char msg[1024];
	strcpy(msg, request);
	char value[20];
	bool accept = true;
	printf("\noutside the loop\n %s",msg);
	for (int i = sizeof msg; i > 0; i--)
	{
		//printf("\ninside the loop\n");
		if (msg[i] == ':' && accept)
		{
			printf("So close");
			int j=0;
			while (msg[i+1]!='\r')
			{
				value[j] = msg[i + 1];
				i++;
				j++;
			}
			value[j] = '\0';
			char *finalstr=new char(j);
			strcpy(finalstr, value);
			printf("\nI  write %s\n", finalstr);
			accept = false;
			i = sizeof msg;
		}
	}
	mysql_init(&conn);
	mysql_real_connect(&conn, "localhost", "Finder", "8154317", "date_for_game", 3306, NULL, 0);
	strcpy(query, "INSERT INTO Date (Coordinates) VALUES ('");
	strcat(query, value);
	strcat(query,"')");
	if (mysql_query(&conn, query) > 0)
	{
		{
			// ���� ���� ������, ...
			printf("%s", mysql_error(&conn));  // ... ������� ��
		}
	}
	mysql_close(&conn);
	//�������� ����� �������
	char Msg[] = "<html><body><h1>STOP</h1></body></html>";
	sender = true;
	senderlock = 0;
	ServerAnswer(my_sock,Msg);
}

//������� �������� ������
void SendData(SOCKET my_sock, char *request)
{
	char msg[1024];
	int locker;
	int locklist[3];//������ ��������, ������� ��� �������� ������
	strcpy(msg, request);
	char value[20];
	bool accept = true;
	printf("\noutside the loop\n %s", msg);
	for (int i = sizeof msg; i > 0; i--)
	{
		//printf("\ninside the loop\n");
		if (msg[i] == ':' && accept)
		{
			printf("So close");
			int j = 0;
			while (msg[i + 1] != '\r')
			{
				value[j] = msg[i + 1];
				i++;
				j++;
			}
			value[j] = '\0';
			char *finalstr = new char(j);
			strcpy(finalstr, value);
			locker = atoi(finalstr);
			printf("\nI accepted %d\n", locker);
			accept = false;
			i = sizeof msg;
		}
	}

	bool finder = false;//����� ������� ��� ��������
	for (int i = 0; i < 3; i++)
	{
		if (locker == locklist[i])
			finder = true;
	}


	if (sender && finder==false)
	{
		bool add = true;//���������� ������� ��� ��������
		for (int i = 0; i < 3; i++)
		{
			if (locklist[i]==0 && add==true)
			{
				locklist[i] = locker;
				add = false;
			}
		}

		MYSQL conn;
		MYSQL_RES *res;
		MYSQL_ROW row;
		char query[100];
		//�������������� ����������
		mysql_init(&conn);
		//����������� � �������� � ����� ������
		mysql_real_connect(&conn, "localhost", "Finder", "8154317", "date_for_game", 3306, NULL, 0);
		//����� ����� �������
		strcpy(query, "SELECT Coordinates FROM Date  WHERE Id=(SELECT MAX(Id) FROM Date);");
		//��������� ������
		if (mysql_query(&conn, query) > 0)
		{
			{
				// ���� ���� ������, ...
				printf("%s", mysql_error(&conn));  // ... ������� ��
			}
		}
		//���� ������ ���, ��������� ������
		else
		{
			senderlock++;
			res = mysql_store_result(&conn);//����� � ������� (�����)
			row=mysql_fetch_row(res);//����������� ������� ����� �������� �������
			char *msg=new char(sizeof row[0]);//����������� ��������� ���������� ���������� ������
			strcpy(msg, row[0]);//����� ������ ������, �.�. ��� ������� ���������� ������� ������� ���� ������ ������
			mysql_free_result(res);//������� ����� ��� ������������ ����������
			ServerAnswer(my_sock, msg);//���������� ������
			if (senderlock == 3)
				sender = false;
		}
		mysql_close(&conn);//��������� ����������
	}
	else
	{
		char Msg[] = "NO";
		ServerAnswer(my_sock, Msg);
	}
	

}


void Handler(SOCKET my_sock, char  *request)
{
	int i;
	bool accept = true;
	char value[1024];
	strcpy(value, request);
	//printf("%s\n\n",value);
	for (i = 0; i < sizeof value && accept; i++)
	{
		if (value[i] == '/')
		{
			if (value[i + 1] == 'r' && value[i + 2]=='e')
			{
				//��������� �������� �� �������� ������
				printf("Sending");
				SendData(my_sock, request);
			}

			if (value[i + 1] == 's' && value[i + 2] == 'e')
			{
				//��������� �������� �� ������ ����� ������ � ��
				printf("Writing");
				WriteData(my_sock, request);
			}
			if (value[i + 1] == 'n' && value[i + 2] == 'u')
			{
				//��������� �������� �� ��������� ������ ������
				char numb[5];
				number++;
				itoa(number, numb, 10);
				ServerAnswer(my_sock, numb);
				//WriteData(my_sock, request);
			}
			accept = false;
			i = sizeof request;
		}
	}
}


//������� ������ � ���������
DWORD WINAPI ToClient(LPVOID client_socket)
{
	//std::string Request;
	char buff[1024];
	SOCKET my_sock;
	my_sock = ((SOCKET *)client_socket)[0];
	int bytes_recv;
	while (true)
	{
		if (bytes_recv = recv(my_sock, buff, sizeof buff, 0) !=SOCKET_ERROR)
			Handler(my_sock,buff);//���������� �������� �������
	}
	return 0;
}

int main(int argc, char *argv[])
{
	
	//setlocale(LC_ALL,"Russian");
	char buff[1024]; // ����� ��� ������
	if (WSAStartup(0x0202, (WSADATA *)&buff[0]))
	{
		// ������!
		printf("Error WSAStartup %d\n", WSAGetLastError());
		return -1;
	}
	// ��� 2 _ �������� ������
	SOCKET mysocket;
	// AF_INET _ ����� ���������
	// SOCK_STREAM _ ��������� ����� (� ���������� ����������)
	// 0 _ �� ��������� ���������� TCP ��������
	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		// ������!
		printf("Error socket %d\n", WSAGetLastError());
		WSACleanup(); // �������������� ���������� Winsock
		return -1;
	}
	// ��� 3 ���������� ������ � ��������� �������
	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(MY_PORT); // �� �������� � ������� �������!!!
	local_addr.sin_addr.s_addr = 0; // ������ ��������� �����������
	// �� ��� ���� IP_������
	// �������� bind ��� ����������
	if (bind(mysocket, (sockaddr *)&local_addr, sizeof(local_addr)))
	{
		// ������
		printf("Error bind %d\n", WSAGetLastError());
		closesocket(mysocket); // ��������� �����!
		WSACleanup();
		return -1;
	}
	// ��� 4 �������� �����������
	// ������ ������� � 0x100
	if (listen(mysocket, 0x100))
	{
		// ������
		printf("Error listen %d\n", WSAGetLastError());
		closesocket(mysocket);
		WSACleanup();
		return -1;
	}
	printf("Wait for conections\n");
	SOCKET client_socket; // ����� ��� �������
	sockaddr_in client_addr; // ����� ������� (����������� ��������)
	// ������� accept ���������� �������� ������ ���������
	int client_addr_size = sizeof(client_addr);
	// ���� ���������� �������� �� ����������� �� ������� (������������� ������)
	while ((client_socket = accept(mysocket, (sockaddr *)&client_addr, &client_addr_size)))
	{
		DWORD thID;
		CreateThread(NULL, NULL, ToClient, &client_socket, NULL, &thID);
		printf("Connect\n");
	}
	getchar();
	return 0;
}