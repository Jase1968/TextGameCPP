#include <iostream>

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include <list>

// SERVER

std::mutex mutexIO;


void writeToConsole(const std::string& text, bool newLine = true)
{
	// locks this scope
	std::unique_lock<std::mutex> lock(mutexIO);

	std::cout << text;
	if (newLine)
		std::cout << std::endl;
}


class Chat
{
public:
	Chat() :
		port(40000),
		connections(0),
		maxConnections(10),
		threadListener(&Chat::listenToClients, this),
		threadSend(&Chat::updateSending, this)
	{
		ip = sf::IpAddress::getLocalAddress();
		if (listener.listen(port) != sf::Socket::Done)
			return;
		selector.add(listener);
		writeToConsole("Server is listening to: " + std::to_string(port));
		startListening = true;
		startSending = true;
	}
	~Chat() {}

	void update()
	{
		// receiving clients message
		while (true)
		{
			for (std::list<std::unique_ptr<sf::TcpSocket>>::iterator it = clients.begin(); it != clients.end(); ++it)
			{
				std::size_t received = 0;
				char* tbuffer = new char[100];
				(*it)->receive(tbuffer, maxMessageSize, received);
				buffer = tbuffer;
				if (received > 0)
				{
					writeToConsole("(" + (*it)->getRemoteAddress().toString() + "):" + buffer);
				}
				delete[] tbuffer;
			}
		}
	}

	void updateSending()
	{
		while (true)
		{
			if (startSending)
			{
				for (std::list<std::unique_ptr<sf::TcpSocket>>::iterator it = clients.begin(); it != clients.end(); ++it)
				{
					std::string output = "this sending123";
					if ((*it)->send(output.c_str(), output.size() + 1) == sf::Socket::Done)
						startSending = false;
				}
			}
		}
	}

	void listenToClients()
	{
		while (true)
		{
			if (startListening)
			{
				// waiting for data on sockets
				writeToConsole("waiting for new connections...");
				if (selector.wait())
				{
					if (selector.isReady(listener))
					{
						std::unique_ptr<sf::TcpSocket> client = std::make_unique<sf::TcpSocket>();
						if (listener.accept(*client) == sf::Socket::Done)
						{
							writeToConsole("client connected (" + client->getRemoteAddress().toString() + ")");

							std::string output = "You connected to port: " + ip.toString();
							client->send(output.c_str(), output.size() + 1);

							// add new client
							clients.push_back(std::move(client));
							selector.add(*clients.back());
						}
					}
					else
					{
						for (std::list<std::unique_ptr<sf::TcpSocket>>::iterator it = clients.begin(); it != clients.end(); ++it)
						{
							sf::TcpSocket& client = **it;
							// client sent data
							if (selector.isReady(client))
							{
								std::size_t received = 0;
								char* tbuffer = new char[100];
								(*it)->receive(tbuffer, maxMessageSize, received);
								buffer = tbuffer;
								if (received > 0)
								{
									writeToConsole("(" + (*it)->getRemoteAddress().toString() + "):" + buffer);
								}

								//sf::Packet packet;

								//if (client.receive(packet) == sf::Socket::Done)
								//{
								//	packet >> buffer;
								//	writeToConsole(buffer);
								//}
								delete[] tbuffer;
							}

						}
					}
				}
			}
		}
	}


private:
	std::string chatLog;
	sf::TcpListener listener;
	sf::SocketSelector selector;
	std::list<std::unique_ptr<sf::TcpSocket>> clients;
	sf::IpAddress ip;
	unsigned short connections;
	unsigned short maxConnections;

	bool startListening{ false };
	bool startSending{ false };

	std::string buffer;
	unsigned short port;
	const unsigned int maxMessageSize = 100;

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