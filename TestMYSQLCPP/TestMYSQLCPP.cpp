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
#include <winsock2.h> // Wincosk2.h должен быть раньше windows!
#include <windows.h>
#include <mysql.h>
#define MY_PORT 80 // Порт, который слушает сервер
#pragma warning(disable:4996)


int number = 0;
bool sender = false;
int senderlock = 0;
int locklist[3];//список клиентов, которые уже получили данные

//Функция ответа сервера на запросы клиентов
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


//Функция добавления новых данных в БД
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
			// Если была ошибка, ...
			printf("%s", mysql_error(&conn));  // ... выведем ее
		}
	}
	mysql_close(&conn);
	//Отправим ответ клиенту
	char Msg[] = "<html><body><h1>STOP</h1></body></html>";
	sender = true;
	senderlock = 0;
	ServerAnswer(my_sock,Msg);
}

//Функция рассылки данных
void SendData(SOCKET my_sock, char *request)
{
	char msg[1024];
	int locker;
	int locklist[3];//список клиентов, которые уже получили данные
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

	bool finder = false;//поиск клиента для отправки
	for (int i = 0; i < 3; i++)
	{
		if (locker == locklist[i])
			finder = true;
	}


	if (sender && finder==false)
	{
		bool add = true;//добавление клиента для отправки
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
		//инициализируем соединение
		mysql_init(&conn);
		//соединяемся с сервером и базой данных
		mysql_real_connect(&conn, "localhost", "Finder", "8154317", "date_for_game", 3306, NULL, 0);
		//пишем текст запроса
		strcpy(query, "SELECT Coordinates FROM Date  WHERE Id=(SELECT MAX(Id) FROM Date);");
		//выполняем запрос
		if (mysql_query(&conn, query) > 0)
		{
			{
				// Если была ошибка, ...
				printf("%s", mysql_error(&conn));  // ... выведем ее
			}
		}
		//Если ошибок нет, считываем данные
		else
		{
			senderlock++;
			res = mysql_store_result(&conn);//буфер с данными (ридер)
			row=mysql_fetch_row(res);//присваиваем массиву строк значение буффера
			char *msg=new char(sizeof row[0]);//присваиваем локальной переменной полученные данные
			strcpy(msg, row[0]);//берем первую строку, т.к. при условии выполнения запроса получим одну строку данных
			mysql_free_result(res);//очищаем ридер для последующего считывания
			ServerAnswer(my_sock, msg);//отправляем данные
			if (senderlock == 3)
				sender = false;
		}
		mysql_close(&conn);//закрываем соединение
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
				//Обработка запросов на отправку данных
				printf("Sending");
				SendData(my_sock, request);
			}

			if (value[i + 1] == 's' && value[i + 2] == 'e')
			{
				//Обработка запросов на запись новых данных в БД
				printf("Writing");
				WriteData(my_sock, request);
			}
			if (value[i + 1] == 'n' && value[i + 2] == 'u')
			{
				//Обработка запросов на получение номера игрока
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


//Функция работы с клиентами
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
			Handler(my_sock,buff);//обработчик запросов клиента
	}
	return 0;
}

int main(int argc, char *argv[])
{
	
	//setlocale(LC_ALL,"Russian");
	char buff[1024]; // буфер для старта
	if (WSAStartup(0x0202, (WSADATA *)&buff[0]))
	{
		// Ошибка!
		printf("Error WSAStartup %d\n", WSAGetLastError());
		return -1;
	}
	// Шаг 2 _ создание сокета
	SOCKET mysocket;
	// AF_INET _ сокет Интернета
	// SOCK_STREAM _ потоковый сокет (с установкой соединения)
	// 0 _ по умолчанию выбирается TCP протокол
	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		// Ошибка!
		printf("Error socket %d\n", WSAGetLastError());
		WSACleanup(); // Деиницилизация библиотеки Winsock
		return -1;
	}
	// Шаг 3 связывание сокета с локальным адресом
	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(MY_PORT); // не забываем о сетевом порядке!!!
	local_addr.sin_addr.s_addr = 0; // сервер принимаем подключения
	// на все свои IP_адреса
	// вызываем bind для связывания
	if (bind(mysocket, (sockaddr *)&local_addr, sizeof(local_addr)))
	{
		// Ошибка
		printf("Error bind %d\n", WSAGetLastError());
		closesocket(mysocket); // закрываем сокет!
		WSACleanup();
		return -1;
	}
	// Шаг 4 ожидание подключений
	// размер очереди – 0x100
	if (listen(mysocket, 0x100))
	{
		// Ошибка
		printf("Error listen %d\n", WSAGetLastError());
		closesocket(mysocket);
		WSACleanup();
		return -1;
	}
	printf("Wait for conections\n");
	SOCKET client_socket; // сокет для клиента
	sockaddr_in client_addr; // адрес клиента (заполняется системой)
	// функции accept необходимо передать размер структуры
	int client_addr_size = sizeof(client_addr);
	// цикл извлечения запросов на подключение из очереди (многопоточный сервер)
	while ((client_socket = accept(mysocket, (sockaddr *)&client_addr, &client_addr_size)))
	{
		DWORD thID;
		CreateThread(NULL, NULL, ToClient, &client_socket, NULL, &thID);
		printf("Connect\n");
	}
	getchar();
	return 0;
}