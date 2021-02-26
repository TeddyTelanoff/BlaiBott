#include <Symple/Net/Server.h>

#include "Common/Common.h"

class Server: public Symple::Net::Server<BlaiMessage>
{
public:
	Server(uint16_t port)
		: Symple::Net::Server<BlaiMessage>(port) {}
protected:
	virtual bool OnClientConnect(std::shared_ptr<Symple::Net::Connection<BlaiMessage>> client) override
	{
		Symple::Net::Message<BlaiMessage> msg;
		msg.Header.Id = BlaiMessage::Welcome;
		msg << client->GetId();
		client->Send(msg);

		return true;
	}

	virtual void OnMessageRecieve(std::shared_ptr<Symple::Net::Connection<BlaiMessage>> client, Symple::Net::Message<BlaiMessage> &msg) override
	{
		std::cout << "Message: " << msg << '\n';

		switch (msg.Header.Id)
		{
		case BlaiMessage::Message:
		{
			MessageAllClients(msg, client);

			uint32_t len;
			msg >> len;
			char *buff;
			if (len > 255)
				buff = new char[len + 1];
			else
				buff = (char *)alloca(len + 1);
			buff[len] = 0;

			for (uint32_t i = len; i >= 1; i--)
				msg >> buff[i - 1];
			// std::cout << "Length: " << len << ", Buffer: " << buff << '\n';

			printf("[Client #%u]: %s\n", client->GetId(), buff);
			break;
		}
		case BlaiMessage::Goodbye:
		{
			client->Disconnect();
			break;
		}
		}
	}
public:
	std::string Answer(std::string_view msg)
	{
		
	}
};

volatile bool s_ShouldExit;

void ConsoleCheckThread()
{
    while (!s_ShouldExit)
    {
        std::string line;
		std::getline(std::cin, line);

		if (line == "/exit" || line == "/stop")
			s_ShouldExit = true;
		else if (line == "/test")
			puts("Test works!");
    }
}

int main()
{
    Server server(BLAI_PORT);
    server.Start();

    std::thread consoleCheckThread(ConsoleCheckThread);

    while (!s_ShouldExit)
        server.Update();

	if (consoleCheckThread.joinable())
		consoleCheckThread.join();
}