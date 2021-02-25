#include <Symple/Net/Client.h>

#include "Common/Common.h"

class Client: public Symple::Net::Client<BlaiMessage>
{
public:
};

volatile bool s_ShouldExit;

void ConsoleCheckThread()
{
    while (!s_ShouldExit)
    {
        std::string line;
		std::getline(std::cin, line);

		if (line == "/exit" || line == "/leave")
			s_ShouldExit = true;
    }
}

int main()
{
    Client client;
    client.Connect("127.0.0.1", BLAI_PORT);
    
    std::thread consoleCheckThread(ConsoleCheckThread);

    while (!(s_ShouldExit |= !client.IsConnected()))
    {
        if (!client.IncomingMessages().IsEmpty())
        {
            auto msg = client.IncomingMessages().PopFront().Message;

            switch (msg.Header.Id)
            {
            case BlaiMessage::Welcome:
                puts("[Server]: Welcome to server!");
                break;
            }
        }
    }

	if (consoleCheckThread.joinable())
		consoleCheckThread.join();
}