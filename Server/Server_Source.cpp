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
	//	��������� sockaddr �������� ����� ���������� ��� ���� �������� ����� � ���������� �������
	//	sockaddr_in � ���� ������� ������������� ��� ������� ��������� IPv4
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


//	������ ������� �������
int WSAStart(WSAData& wsaData)
{
	int initCode{ 0 };

	//	������ �������� - �������� ��������� ������ ���������� �������, ������� �� ����� ������������.
	//	������ �������� - ������ �� ��������� WSADATA, � ������� ��� ������������� ������������ ����������
	//	� ������� ������ ���������� ������� �� ������ ������.
	initCode = WSAStartup(MAKEWORD(2, 2), &wsaData);

	//	� ������ ������ ������� ���������� �������� 0, � ������ ������ - ����������� ���
	//
	// "����� ������� WSAGetLastError �� ��������� � �� ������ ��������������."
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

//	�������� � ������������� ������
int SocketInitialize(SOCKET& _socket)
{
	//	������������� ��������������� ������ ������ � ��������� � �������� ���������� ������������ ����
	//	AF_INET ���������� ��������� ������� IPv4
	//	SOCK_STREAM ���������� ��� ������ ��� TCP
	//	������ �������� ���������� ������������ �������� ��� ������������ ��������� ������� � ���� ������
	//	� ����� ������ ����� ������������ �������� 0, ����� ��������� ������������� ��������� ����������� ��������
	_socket = socket(AF_INET, SOCK_STREAM, NULL);

	//	� ������ ������ ������� SOCKET() ���������� ����������, ����������� �� ����� �����
	//	� ��������� ������ ���������� �������� INVALID_SOCKET, ����������� ��� ������ �������� � ������� ������� WSAGetLastError()
	//	���� ������� socket() ��������� � �������, �� ��� ������������� �������� ������� �������� ������
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

//	�������� ���������� �������� �����:���� � ������������� ������, ����� � ���� ����� ���� ���������� �� ������� �������
int BindListenSocket(SOCKET& _socket, char* ip, sockaddr_in& serverInfo)
{
	//	��������� in_addr �������� ������ �������������� �������� ��������� IPv4 ������
	in_addr ip_to_num;

	int initCode{ 0 };

	//	� ������� ������� inet_pton ����������� ����� �� ������� char[] � in_addr
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

	//	���������� ��������� ���� ��������� sockaddr_in, � ������� ������� �� ������ ����� � ����� �����:����
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr = ip_to_num;
	serverInfo.sin_port = htons(1000);

	//	�������� ������ � ���������� ���� �����:����
	//	�� ��������� undefined behavior ���������� ���������� ���� ��������� sockaddr_in � ������ ������������� ������ IPv4
	//	����� ���� ������������� ��� � ����� ��� sockaddr
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

//	������ "�������������" �������� ���������� �� ������� �������
int SetListen(SOCKET& _socket)
{
	int initCode{ 0 };

	//	������� listen �������� ����� � ��������� � ������� �� ������������ �������� ����������
	//	� ������ ������ ���������� 0
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

//	�������� ��������� ���������� �� ������� �������
SOCKET AcceptConnection(SOCKET& _socket, sockaddr_in& clientInfo)
{
	int clientInfo_sz = sizeof(clientInfo);
	//	������ ��������� ����� ��� ������ �������
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
