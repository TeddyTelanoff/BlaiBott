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
		client->Send(msg);

		return true;
	}
public:
	
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
    }
}

int main()
{
    Server server(BLAI_PORT);
    server.Start();

    std::thread consoleCheckThread(ConsoleCheckThread);

    while (!s_ShouldExit)
        server.Update(false);

	if (consoleCheckThread.joinable())
		consoleCheckThread.join();
}