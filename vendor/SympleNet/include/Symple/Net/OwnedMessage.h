#pragma once

#include "Symple/Net/Message.h"

namespace Symple::Net
{
	template<typename T>
	class Connection;

	template<typename T>
	struct OwnedMessage final
	{
		std::shared_ptr<Connection<T>> Remote {};
		Message<T> Message {};

		friend std::ostream &operator <<(std::ostream &os, const OwnedMessage &msg)
		{ return os << msg.Message; }
	};
}