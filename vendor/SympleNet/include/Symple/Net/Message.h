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
		Message &operator <<(const DataType &data)
		{
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to serialize");

			// Grow and copy data into body
			size_t i = Body.size();
			Body.resize(Body.size() + sizeof(DataType));
			std::memcpy(Body.data() + i, &data, sizeof(DataType));

			Header.Size = Size();
			return *this;
		}

		template<>
		Message &operator <<(const char *const &data)
		{
			uint32_t sz = 0;
			const char *p = data;
			if (*p)
			{
				*this << *p;
				while (*++p && ++sz)
					*this << *p;
			}
			return *this << sz;
		}

		template<>
		Message &operator <<(const std::string &data)
		{
			for (const char &c : data)
				*this << c;
			return *this << uint32_t(data.size());;
		}

		template<>
		Message &operator <<(const std::string_view &data)
		{
			for (const char &c : data)
				*this << c;
			return *this << uint32_t(data.size());;
		}

		template<typename DataType>
		Message &operator >>(DataType &data)
		{
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to serialize");

			// Copy body into data
			size_t i = Body.size() - sizeof(DataType);
			std::memcpy(&data, Body.data() + i, sizeof(DataType));

			Body.resize(i);
			Header.Size = Size();
			return *this;
		}

		template<>
		Message &operator >>(const char* &data)
		{
			uint32_t sz;
			*this >> sz;
			char *buff = new char[sz + 2];
			for (uint32_t i = sz + 1; i; i--)
				*this >> buff[i - 1];
			buff[sz + 1] = 0;
			data = buff;
			return *this;
		}

		template<>
		Message &operator >>(std::string &data)
		{
			uint32_t sz;
			*this >> sz;
			data.resize(sz);
			char *buff = data.data();
			for (uint32_t i = 0; i < sz; i++)
				*this >> buff[sz - i - 1];
			return *this << (char)0;
		}

		template<>
		Message &operator >>(std::string_view &data)
		{
			uint32_t sz;
			*this >> sz;
			char *buff = new char[sz];
			for (uint32_t i = 0; i < sz; i++)
				*this >> buff[sz - i - 1];
			data = { buff, sz };
			return *this;
		}
	};
}