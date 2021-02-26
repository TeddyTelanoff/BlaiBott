#include <Symple/Net/Client.h>

#include "Common/Common.h"
#undef SendMessage


class Client: public Symple::Net::Client<BlaiMessage>
{
public:
    uint32_t m_Id;
public:
    void SendMessage(const std::string &str)
    {
        Symple::Net::Message<BlaiMessage> msg;
        msg.Header.Id = BlaiMessage::Message;
        for (auto c : str)
            msg << c;
        msg << (uint32_t)str.length();
        
        // std::cout << "Sending msg \"" << str << "\"\n";
        Send(msg);

        // uint32_t len;
        // msg >> len;
        // std::cout << "Length = " << len << '\n';
    }

    void SendGoodbye()
    {
        Symple::Net::Message<BlaiMessage> msg;
        msg.Header.Id = BlaiMessage::Goodbye;
        Send(msg);
    }
};

Client s_Client;
volatile bool s_ShouldExit;

void ConsoleCheckThread()
{
    while (!s_ShouldExit)
    {
        printf("[Client #%u]: ", s_Client.m_Id);

        std::string line;
		std::getline(std::cin, line);

		if (line == "/exit" || line == "/leave")
        {
			s_Client.SendGoodbye();
            break;
        }
		else if (line == "/test")
			puts("Test works!");
        else
        {
            s_Client.SendMessage(line);
        }
    }
}

int main()
{
    s_Client.Connect("127.0.0.1", BLAI_PORT);
    
    std::thread consoleCheckThread(ConsoleCheckThread);

    while (!(s_ShouldExit |= !s_Client.IsConnected()))
    {
        if (!s_Client.IncomingMessages().IsEmpty())
        {
            auto msg = s_Client.IncomingMessages().PopFront().Message;

            switch (msg.Header.Id)
            {
            case BlaiMessage::Welcome:
                puts("[Server]: Welcome to server!");
                msg >> s_Client.m_Id;
                break;
            }
        }
    }

	if (consoleCheckThread.joinable())
		consoleCheckThread.join();
}