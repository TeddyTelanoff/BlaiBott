#pragma once

#include "Symple/Net/Connection.h"
#include "Symple/Net/ThreadSafeQueue.h"

namespace Symple::Net
{
	template<typename T>
	class Server
	{
	protected:
		ThreadSafeQueue<OwnedMessage<T>> m_RecievedMessages;
		std::deque<std::shared_ptr<Connection<T>>> m_Connections;

		asio::io_context m_AsioContext;
		std::thread m_ContextThread;

		asio::ip::tcp::acceptor m_AsioAcceptor;
		uint32_t m_IdCounter = 0;
	public:
		Server(uint16_t port)
			: m_AsioAcceptor(m_AsioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
		{ }

		virtual ~Server()
		{ Stop(); }

		bool Start()
		{
			try
			{
				WaitForClientConnection();
				m_ContextThread = std::thread([this]() { m_AsioContext.run(); });

				std::cout << "[!]<Server>: Started!\n";
				return true;
			}
			catch (std::exception &e)
			{
				std::cerr << "[!]<Server> Exception: " << e.what() << '\n';
				return false;
			}
		}

		void Stop()
		{
			m_AsioContext.stop();
			if (m_ContextThread.joinable())
				m_ContextThread.join();

			std::cerr << "[!]<Server>: Stopped!\n";
		}

		[[async]] void WaitForClientConnection()
		{
			m_AsioAcceptor.async_accept(
				[this](std::error_code ec, asio::ip::tcp::socket socket)
				{
					if (ec)
						std::cerr << "[!]<Server> New connection error: " << ec.message() << '\n';
					else
					{
						std::cout << "[$]<Server> New connection: " << socket.remote_endpoint() << '\n';

						std::shared_ptr<Connection<T>> client = std::make_shared<Connection<T>>(Connection<T>::Owner::Server,
							m_AsioContext, std::move(socket), m_RecievedMessages);

						if (OnClientConnect(client))
						{
							m_Connections.push_back(std::move(client));
							m_Connections.back()->ConnectToClient(m_IdCounter++);

							std::cout << "[$]<Client #" << m_Connections.back()->GetId() << ">: Connection approved!\n";
						}
						else
							std::cout << "[$]: Connection denied!\n";
					}

					WaitForClientConnection();
				});
		}

		void MessageClient(std::shared_ptr<Connection<T>> client, const Message<T> &msg)
		{
			if (client && client->IsConnected())
				client->Send();
			else
			{
				OnClientDisconnect(client);
				client.reset();
				m_Connections.erase(
					std::remove(m_Connections.begin(), m_Connections.end(), client), m_Connections.end());
			}
		}

		void MessageAllClients(const Message<T> &msg, std::shared_ptr<Connection<T>> ignore = nullptr)
		{
			bool invalidClientExists = false;

			for (auto &client : m_Connections)
			{
				if (client && client->IsConnected())
				{
					if (client != ignore)
						client->Send(msg);
				}
				else
				{
					OnClientDisconnect(client);
					client.reset();
					invalidClientExists = true;
				}
			}

			if (invalidClientExists)
				m_Connections.erase(
					std::remove(m_Connections.begin(), m_Connections.end(), nullptr), m_Connections.end());
		}

		void Update(bool wait = true, size_t maxMessages = -1)
		{
			if (wait)
				m_RecievedMessages.Wait();

			size_t msgCount = 0;
			while (msgCount < maxMessages && !m_RecievedMessages.IsEmpty())
			{
				auto msg = m_RecievedMessages.PopFront();
				OnMessageRecieve(msg.Remote, msg.Message);
				msgCount++;
			}
		}
	protected:
		virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client)
		{ return true; }

		virtual void OnClientDisconnect(std::shared_ptr<Connection<T>> client)
		{ }

		virtual void OnMessageRecieve(std::shared_ptr<Connection<T>> client, Message<T> &msg)
		{ }
	};
}