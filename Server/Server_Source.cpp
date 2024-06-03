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
int SocketInitialize(SOCKET& _socket);
int BindListenSocket(SOCKET& _socket, char* ip, sockaddr_in& serverInfo);
int SetListen(SOCKET& _socket);
SOCKET AcceptConnection(SOCKET& _socket, sockaddr_in& clientInfo);
int HandleClient(SOCKET& _clientSocket, SOCKET& listenSocket);

int main()
{
	WSAData wsaData;
	SOCKET listeningSocket;
	//	Структура sockaddr является общей структурой для всех вариаций типов и протоколов сокетов
	//	sockaddr_in в свою очередь предназначена для сокетов семейства IPv4...
	sockaddr_in serverInfo;
	sockaddr_in clientInfo;
	ZeroMemory(&serverInfo, sizeof(serverInfo));
	ZeroMemory(&clientInfo, sizeof(clientInfo));

	char ip[]{ "127.0.0.1" };


	if (WSAStart(wsaData) != 0) { return 1; }

	if (SocketInitialize(listeningSocket) != 0) { return 1; }

	if (BindListenSocket(listeningSocket, ip, serverInfo) != 0) { return 1; }

	if (SetListen(listeningSocket) != 0) { return 1; }

	SOCKET clientSocket = AcceptConnection(listeningSocket, clientInfo);
	if (clientSocket == INVALID_SOCKET) { return 1; }

	while (true)
	{
		if(HandleClient(clientSocket, listeningSocket) != 0)
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

//	Привязка конкретных значений адрес:порт к существующему сокету, чтобы к нему можно было обратиться со стороны клиента
int BindListenSocket(SOCKET& _socket, char* ip, sockaddr_in& serverInfo)
{
	//	Структура in_addr является особым представлением обычного строчного IPv4 адреса
	in_addr ip_to_num;

	int initCode{ 0 };

	//	С помощью функции inet_pton преобразуем адрес из формата char[] в in_addr
	initCode = inet_pton(AF_INET, ip, &ip_to_num);
	if (initCode != 1)
	{
		cout << "IP translation to special numeric format failed with code: " << WSAGetLastError() << endl;
		closesocket(_socket);
		WSACleanup();

		return 1;
	}
	else
		cout << "IP translation succesful" << endl;

	//	Необходимо присвоить поля структуре sockaddr_in, с помощью которой мы свяжем сокет с парой адрес:порт
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr = ip_to_num;
	serverInfo.sin_port = htons(1000);

	//	Привязка сокета к конкретной паре адрес:порт
	//	Во избежание undefined behavior необходимо определить поля структуры sockaddr_in в случае использования сокета IPv4
	//	После чего преобразовать его в общий тип sockaddr
	//	https://stackoverflow.com/questions/29649348/socket-programming-struct-sockaddr-vs-struct-sockaddr-in
	//
	initCode = bind(_socket, (sockaddr*)&serverInfo, sizeof(serverInfo));
	if (initCode == SOCKET_ERROR)
	{
		cout << "Socket binding failed with code: " << WSAGetLastError() << endl;
		closesocket(_socket);
		WSACleanup();

		return 1;
	}
	else
		cout << "Socket binded succesfully" << endl;

	return 0;
}

//	Начало "прослушивания" входящих соединений со стороны клиента
int SetListen(SOCKET& _socket)
{
	int initCode{ 0 };

	//	Функция listen помещает сокет в состояние в котором он прослушивает входящее соединение
	//	В случае успеха возвращает 0
	initCode = listen(_socket, SOMAXCONN);
	if (initCode == SOCKET_ERROR)
	{
		cout << "Listening failed with code: " << WSAGetLastError() << endl;
		closesocket(_socket);
		WSACleanup();

		return 1;
	}
	else
		cout << "Listening..." << endl;

	return 0;
}

//	Принятие входящего соединения со стороны клиента
SOCKET AcceptConnection(SOCKET& _socket, sockaddr_in& clientInfo)
{
	int clientInfo_sz = sizeof(clientInfo);
	//	Создаём отдельный сокет для нового клиента
	SOCKET clientSocket = accept(_socket, (sockaddr*)&clientInfo, &clientInfo_sz);
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "Client detected, connection failed with code: " << WSAGetLastError() << endl;
		closesocket(clientSocket);
		closesocket(_socket);
		WSACleanup();

		return INVALID_SOCKET;
	}
	cout << "Client detected, connection established" << endl;
	
	return clientSocket;
}

//
int HandleClient(SOCKET& _clientSocket, SOCKET& listenSocket)
{
	const int BUFF_SIZE{ 1024 };
	std::vector<char> clientMessage(BUFF_SIZE);
	std::vector<char> serverResponse(BUFF_SIZE);
	int packet_size{ 0 };

	packet_size = recv(_clientSocket, clientMessage.data(), clientMessage.size(), NULL);
	if (packet_size == SOCKET_ERROR)
	{
		cout << "Client message handle failed with code: " << WSAGetLastError() << endl;
		closesocket(_clientSocket);
		closesocket(listenSocket);
		WSACleanup();

		return 1;
	}
	else if (clientMessage[0] == 'e' && clientMessage[1] == 'x' && clientMessage[2] == 't')
	{
		cout << "Client disconnected " << endl;
		closesocket(_clientSocket);
		closesocket(listenSocket);
		WSACleanup();

		return 2;
	}

	cout << "Enter: ";
	fgets(serverResponse.data(), serverResponse.size(), stdin);

	packet_size = send(_clientSocket, serverResponse.data(), serverResponse.size(), NULL);
	if (packet_size == SOCKET_ERROR)
	{
		cout << "Client message handle failed with code: " << WSAGetLastError() << endl;
		closesocket(_clientSocket);
		closesocket(listenSocket);
		WSACleanup();

		return 1;
	}

	return 0;
}
