#pragma once

#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <stdio.h>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")

using std::cout, std::endl;


int WSAStart(WSAData& wsaData);
int SocketInitialize( SOCKET &_socket);
int ConnectToServer(SOCKET& _socket, sockaddr_in& serverInfo, char* server_ip);
int ServerRequest(SOCKET& _socket);

int main()
{
	WSAData wsaData;
	SOCKET connectionSocket;
	sockaddr_in serverInfo;
	ZeroMemory(&serverInfo, sizeof(serverInfo));

	char server_ip[]{ "127.0.0.1" };

	if (WSAStart(wsaData) != 0) { return 1; }

	if (SocketInitialize(connectionSocket) != 0) { return 1; }

	if (ConnectToServer(connectionSocket, serverInfo, server_ip) != 0) { return 1; }

	while (true)
	{
		if (ServerRequest(connectionSocket) != 0)
			break;
	}

	return 0;
}


//	Запуск системы сокетов
int WSAStart(WSAData& wsaData)
{
	int initCode{ 0 };

	//	Первый аргумент - указание диапазона версий реализации сокетов, которые мы хотим использовать.
	//	Второй аргумент - ссылка на структуру WSADATA, в которую при инициализации подгружается информация
	//	о текущей версии реализации сокетов на данной машине.
	initCode = WSAStartup(MAKEWORD(2, 2), &wsaData);

	//	В случае успеха функция возвращает значение 0, в случае ошибки - расширенный код
	//
	// "Вызов функции WSAGetLastError не требуется и не должен использоваться."
	//	https://learn.microsoft.com/ru-ru/windows/win32/api/winsock2/nf-winsock2-wsastartup
	if (initCode != 0)
	{
		cout << "WSAStartup failed with code: " << initCode << endl;

		return 1;
	}
	else
		cout << "WSAStartup succesful" << endl;
	return 0;
}

//	Создание и инициализация сокета
int SocketInitialize(SOCKET& _socket)
{
	//	Инициализация непосредственно самого сокета с привязкой к заданным параметрам транспортной сети
	//	AF_INET обозначает семейство адресов IPv4
	//	SOCK_STREAM определяет тип сокета как TCP
	//	Третий параметр определяет используемый протокол для определённого семейства адресов и типа сокета
	//	В общем случае можно использовать значение 0, тогда поставщик автоматически определит необходимое значение
	_socket = socket(AF_INET, SOCK_STREAM, NULL);

	//	В случае успеха функция SOCKET() возвращает дискриптор, ссылающийся на новый сокет
	//	В противном случае возвращает значение INVALID_SOCKET, определённый код ошибки получаем с помощью функции WSAGetLastError()
	//	Если функция socket() завершена с ошибкой, то нет необходимости вызывать функцию закрытия сокета
	if (_socket == INVALID_SOCKET)
	{
		cout << "New socket initialization failed with code: " << WSAGetLastError() << endl;
		WSACleanup();

		return 1;
	}
	else
		cout << "New socket initialization succesful" << endl;

	return 0;
}

//	Создание соединения с сокетом сервера
int ConnectToServer(SOCKET& _socket, sockaddr_in& serverInfo, char* server_ip)
{
	in_addr ip_to_num;
	int initCode{ 0 };

	//	С помощью функции inet_pton преобразуем адрес из формата char[] в in_addr
	initCode = inet_pton(AF_INET, server_ip, &ip_to_num);
	if (initCode != 1)
	{
		cout << "IP translation to special numeric format failed with code: " << WSAGetLastError() << endl;
		closesocket(_socket);
		WSACleanup();

		return 1;
	}
	else
		cout << "IP translation succesful" << endl;

	//	Присваиваем структуре sockaddr_in информацию о сервере для подключения
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr = ip_to_num;
	serverInfo.sin_port = htons(1000);

	//	Установка подключения
	initCode = connect(_socket, (sockaddr*)&serverInfo, sizeof(serverInfo));
	if (initCode == SOCKET_ERROR)
	{
		cout << "Connection to server failed with code: " << WSAGetLastError() << endl;
		closesocket(_socket);
		WSACleanup();

		return 1;
	}
	else
		cout << "Conection established!" << endl;

	return 0;
}

//	Отправка сообщения серверу и получение ответа
int ServerRequest(SOCKET& _socket)
{
	const int BUFF_SIZE{ 1024 };
	std::vector<char> message(BUFF_SIZE);
	std::vector<char> response(BUFF_SIZE);
	int packet_size{ 0 };

	cout << "Enter request: ";
	fgets(message.data(), message.size(), stdin);

	packet_size = send(_socket, message.data(), message.size(), NULL);
	if (packet_size == SOCKET_ERROR)
	{
		cout << "Request failed with code: " << WSAGetLastError() << endl;
		closesocket(_socket);
		WSACleanup();

		return 1;
	}

	packet_size = recv(_socket, response.data(), response.size(), NULL);
	if (packet_size == SOCKET_ERROR)
	{
		cout << "Request failed with code: " << WSAGetLastError() << endl;
		closesocket(_socket);
		WSACleanup();

		return 1;
	}
	else
		cout << "Server response: " << response.data() << endl;

	return 0;
}