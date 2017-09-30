#include <iostream>

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <thread>
#include <mutex>

#include "CBLib.hpp"

// SERVER

std::mutex mutexIO;


void writeToConsole(const std::string& text, bool newLine = true)
{
	// locks this scope
	std::unique_lock<std::mutex> lock(mutexIO);

	std::cout << text;
	if (newLine)
		std::cout<< std::endl;
}


class Chat
{
public:
	Chat() :
		port(40000),
		connections(0),
		threadListener(&Chat::listenToClients, this),
		threadSend(&Chat::updateSending, this)

	{
		ip = sf::IpAddress::getLocalAddress();
		if (listener.listen(port) != sf::Socket::Done)
			return;
		writeToConsole("Server is listening to: " + std::to_string(port));
		startListening = true; 
		startSending = true;
	}
	~Chat(){
	}

	void update()
	{
		// receiving clients message
		while (true)
		{
			std::size_t received = 0;
			client.receive(buffer, sizeof(buffer), received);
			if (received > 0)
			{
				writeToConsole("(" + client.getRemoteAddress().toString() + "):" + buffer);
			}
		}
	}

	void updateSending()
	{
		while (true)
		{
			if (startSending)
			{
				std::string output = "this sending";
				if (client.send(output.c_str(), output.size() + 1) == sf::Socket::Done)
					startSending = false;
			}
		}
	}

	void listenToClients()
	{
		while (true)
		{
			if (startListening)
			{
				writeToConsole("wait for connections");
				if (listener.accept(client) == sf::Socket::Done)
				{
					writeToConsole("client connected (" + client.getRemoteAddress().toString() + ")");

					std::string output = "You connected to port: " + ip.toString();
					client.send(output.c_str(), output.size() + 1);
				}
			}
		}
	}


private:
	std::string chatLog;
	sf::TcpListener listener;
	sf::TcpSocket client;
	sf::IpAddress ip;
	int connections; 
	bool startListening{ false };
	bool startSending{ false };

	char buffer[100];
	unsigned short port;

	std::thread threadListener, threadSend, threadReceive;
};

int main()
{
	Chat chat;

	while (true)
	{
		chat.update();
	}

	return 0;
}