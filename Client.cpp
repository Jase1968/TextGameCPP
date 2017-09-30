#include <iostream>

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <thread>
#include <mutex>

#include "CBLib.hpp"



std::mutex mutexIO;


void writeToConsole(const std::string& text, bool newLine = true)
{
	// locks this scope
	std::unique_lock<std::mutex> lock(mutexIO);

	std::cout << text;
	if (newLine)
		std::cout << std::endl;
}


// CLIENT

class Chat
{
public:
	Chat()
		: 
		threadReceive(&Chat::updateReceiving, this), 
		threadSend(&Chat::updateSending, this),
		port(40000)
		   
	{
	}
	~Chat() {}

	void start()
	{
		do
		{
			writeToConsole(ipAddress.getLocalAddress().toString());
			writeToConsole("Enter ip adress of the server");
			std::cin >> ipAddress;
			ipAddress = ipAddress.getLocalAddress();
			if (client.connect(ipAddress, port) != sf::Socket::Status::Done)
			{
				writeToConsole("couldn't connect to the server. " + std::to_string(port));
			}
			else
			{
				writeToConsole("connected to the server. " + std::to_string(port));
				connected = true;
			}
		} while (!connected);



		while (!quit)
		{
			updateSending();
		}
	}


	void updateSending()
	{
		if (connected)
		{
			std::cin.getline(message, sizeof(message));
			plainText = message;
			checkMessagePattern();
		}
	}
	
	void updateReceiving()
	{
		while (!quit)
		{
			if (connected)
			{
				std::size_t received = 0;

				if (client.receive(message, sizeof(message), received) != sf::Socket::Done)
					continue;

				// if anything is received
				if (received > 0)
				{
					writeToConsole("received: ", false);
					writeToConsole(message);
				}
			}
		}
	}


	void checkMessagePattern()
	{
		if (plainText.size() > 100)
		{
			writeToConsole("message too long.");
		}
		else
		{
			if (client.send(message, sizeof(message)) == sf::Socket::Status::Done)
			{
			}
		}
	}

private:
	sf::TcpSocket client;
	sf::TcpListener listener;

	sf::IpAddress ipAddress;
	bool connected{ false };
	bool quit{ false };
	std::string plainText;
	char message[100];
	unsigned short port;

	std::thread threadSend, threadReceive;

};

int main()
{
	Chat chat;

	chat.start();

	return 0;
}