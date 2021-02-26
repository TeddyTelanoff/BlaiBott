#pragma once

#include "Symple/Net/OwnedMessage.h"
#include "Symple/Net/ThreadSafeQueue.h"

namespace Symple::Net
{
	template<typename T>
	class Server;

	template<typename T>
	class Connection final: public std::enable_shared_from_this<Connection<T>>
	{
	public: enum class Owner;
	private:
		asio::ip::tcp::socket m_Socket;
		asio::io_context &m_AsioContext;

		ThreadSafeQueue<Message<T>> m_MessagesToSend;
		ThreadSafeQueue<OwnedMessage<T>> &m_RecievedMessages;

		Owner m_Owner;
		uint32_t m_Id = 0;
		Message<T> m_TempMsg;

		uint64_t m_HandshakeIn;
		uint64_t m_HandshakeOut;
		uint64_t m_HandshakeCheck;
		ScrambleFunction m_Scramble;
	public:
		Connection(Owner owner,
			asio::io_context &asioContext, asio::ip::tcp::socket socket, ThreadSafeQueue<OwnedMessage<T>> &recievedMessages,
				ScrambleFunction scrambleFn = Scramble)
			: m_Owner(owner),
				m_AsioContext(asioContext), m_Socket(std::move(socket)), m_RecievedMessages(recievedMessages),
					m_Scramble(scrambleFn)
		{
			if (m_Owner == Owner::Server)
			{
				m_HandshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
				m_HandshakeCheck = m_Scramble(m_HandshakeOut);
				#if defined(SY_NET_SHOW_VALIDATION)
				std::cout << "[$]<Server>: Validation key: { In: " << m_HandshakeOut << ", Out: " << m_HandshakeCheck << " }";
				#endif
			}
			else
			{
				m_HandshakeIn = 0;
				m_HandshakeOut = 0;
			}
		}

		~Connection()
		{ }

		uint32_t GetId() const
		{ return m_Id; }

		void ConnectToClient(Server<T> *server, uint32_t id = 0)
		{
			if (m_Owner == Owner::Server && IsConnected())
			{
				m_Id = id;

				WriteValidation();
				ReadValidation(server);
			}
		}

		void ConnectToServer(const asio::ip::tcp::resolver::results_type &endpoints)
		{
			if (m_Owner == Owner::Client)
			{
				asio::async_connect(m_Socket, endpoints,
					[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
					{
						if (!ec)
							ReadValidation();
					});
			}
		}

		void Disconnect()
		{
			if (IsConnected())
				asio::post(m_AsioContext, [this]() { m_Socket.close(); });
		}

		bool IsConnected() const
		{ return m_Socket.is_open(); }

		void Send(const Message<T> &msg)
		{
			asio::post(m_AsioContext,
				[this, msg]()
				{
					bool isWriting = !m_MessagesToSend.IsEmpty();
					m_MessagesToSend.PushBack(msg);
					if (!isWriting)
						WriteHeader();
				});
		}
	private:
		[[async]] void ReadHeader()
		{
			asio::async_read(m_Socket, asio::buffer(&m_TempMsg.Header, sizeof(MessageHeader<T>)),
				[this](std::error_code ec, std::size_t len)
				{
					if (ec)
					{
						#if defined(SY_NET_ENABLE_LOGGING)
						std::cerr << "[!]<Client #" << m_Id << ">: Failed to read header.\n";
						#endif
						m_Socket.close();
					}
					else
						if (m_TempMsg.Header.Size > 0)
						{
							m_TempMsg.Body.resize(m_TempMsg.Header.Size);
							ReadBody();
						}
						else
							AddToIncomingMessageQueue();
				});
		}

		[[async]] void ReadBody()
		{
			asio::async_read(m_Socket, asio::buffer(m_TempMsg.Body.data(), m_TempMsg.Body.size()),
				[this](std::error_code ec, std::size_t len)
				{
					if (ec)
					{
						#if defined(SY_NET_ENABLE_LOGGING)
						std::cerr << "[!]<Client #" << m_Id << ">: Failed to read body.\n";
						#endif
						m_Socket.close();
					}
					else
						AddToIncomingMessageQueue();
				});
		}

		[[async]] void WriteHeader()
		{
			asio::async_write(m_Socket, asio::buffer(&m_MessagesToSend.Front().Header, sizeof(MessageHeader<T>)),
				[this](std::error_code ec, std::size_t len)
				{
					if (ec)
					{
						#if defined(SY_NET_ENABLE_LOGGING)
						std::cerr << "[!]<Client #" << m_Id << ">: Failed to write header.\n";
						#endif
						m_Socket.close();
					}
					else
						if (m_MessagesToSend.Front().Body.size() > 0)
							WriteBody();
						else
						{
							m_MessagesToSend.PopFront();
							if (!m_MessagesToSend.IsEmpty())
								WriteHeader();
						}
				});
		}

		[[async]] void WriteBody()
		{
			asio::async_write(m_Socket, asio::buffer(m_MessagesToSend.Front().Body.data(), m_MessagesToSend.Front().Body.size()),
				[this](std::error_code ec, std::size_t len)
				{
					if (ec)
					{
						#if defined(SY_NET_ENABLE_LOGGING)
						std::cerr << "[!]<Client #" << m_Id << ">: Failed to write body.\n";
						#endif
						m_Socket.close();
					}
					else
					{
						m_MessagesToSend.PopFront();
						if (!m_MessagesToSend.IsEmpty())
							WriteHeader();
					}
				});
		}

		[[async]] void AddToIncomingMessageQueue()
		{
			if (m_Owner == Owner::Server)
				m_RecievedMessages.PushBack({ this->shared_from_this(), m_TempMsg });
			else
				m_RecievedMessages.PushBack({ nullptr, m_TempMsg });

			ReadHeader();
		}

		[[async]] void WriteValidation()
		{
			asio::async_write(m_Socket, asio::buffer(&m_HandshakeOut, sizeof(uint64_t)),
				[this](std::error_code ec, std::size_t len)
				{
					if (ec)
						m_Socket.close();
					else
					{
						if (m_Owner == Owner::Client)
							ReadHeader();
					}
				});
		}

		[[async]] void ReadValidation(Server<T> *server = nullptr)
		{
			asio::async_read(m_Socket, asio::buffer(&m_HandshakeIn, sizeof(uint64_t)),
				[this, server](std::error_code ec, std::size_t len)
				{
					if (ec)
					{
						#if defined(SY_NET_ENABLE_LOGGING)
						std::cerr << "[!]<Server>: Client disconnected while validating.\n";
						#endif
						m_Socket.close();
					}
					else
						if (m_Owner == Owner::Server)
						{
							if (m_HandshakeIn == m_HandshakeCheck)
							{
								#if defined(SY_NET_ENABLE_LOGGING)
								std::cout << "[!]<Server>: Client validated!\n";
								#endif
								server->OnClientValidated(this->shared_from_this());

								ReadHeader();
							}
							else
							{
								#if defined(SY_NET_ENABLE_LOGGING)
								std::cerr << "[!]<Server>: Client (" << m_Socket.remote_endpoint() << ") failed validation.\n";
								#endif
								m_Socket.close();
							}
						}
						else
						{
							m_HandshakeOut = m_Scramble(m_HandshakeIn);
							#if defined(SY_NET_SHOW_VALIDATION)
							std::cout << "[$]<Client>: Validation key: { In: " << m_HandshakeIn << ", Out: " << m_HandshakeOut << " }";
							#endif
							WriteValidation();
						}
				});
		}
	public:
		enum class Owner
		{
			Server,
			Client,
		};
	};
}