#include <Symple/Net/Client.h>

#include "Common/Common.h"
#undef SendMessage

class Client: public Symple::Net::Client<BlaiMessage>
{
public:
    void SendMessage(const std::string &str)
    {
        Symple::Net::Message<BlaiMessage> msg;
        for (auto c : str)
            msg << c;
        msg << (uint32_t)str.length();
        
        // std::cout << "Sending msg \"" << str << "\"\n";
        Send(msg);

        // uint32_t len;
        // msg >> len;
        // std::cout << "Length = " << len << '\n';
    }
};

Client client;
volatile bool s_ShouldExit;

void ConsoleCheckThread()
{
    while (!s_ShouldExit)
    {
        std::string line;
		std::getline(std::cin, line);

		if (line == "/exit" || line == "/leave")
			s_ShouldExit = true;
		else if (line == "/test")
			puts("Test works!");
        else
        {
            client.SendMessage(line);
        }
    }
}

int main()
{
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