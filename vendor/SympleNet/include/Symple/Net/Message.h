#pragma once

#include "Symple/Net/MessageHeader.h"

namespace Symple::Net
{
	template<typename T>
	struct Message final
	{
		MessageHeader<T> Header {};
		std::vector<uint8_t> Body {};

		size_t Size() const
		{ return Body.size(); }
		
		friend std::ostream &operator <<(std::ostream &os, const Message &msg)
		{ return os << "ID: " << uint32_t(msg.Header.Id) << " Size: " << msg.Header.Size; }

		template<typename DataType>
		friend Message &operator <<(Message &msg, const DataType &data)
		{
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to serialize");

			// Grow and copy data into body
			size_t i = msg.Body.size();
			msg.Body.resize(msg.Body.size() + sizeof(DataType));
			std::memcpy(msg.Body.data() + i, &data, sizeof(DataType));

			msg.Header.Size = msg.Size();
			return msg;
		}

		template<typename DataType>
		friend Message &operator >>(Message &msg, DataType &data)
		{
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to serialize");

			// Copy body into data
			size_t i = msg.Body.size() - sizeof(DataType);
			std::memcpy(&data, msg.Body.data() + i, sizeof(DataType));

			msg.Body.resize(i);
			msg.Header.Size = msg.Size();
			return msg;
		}
	};
}