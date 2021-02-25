#pragma once

#include "Symple/Net/Common.h"

namespace Symple::Net
{
	template<typename T>
	struct MessageHeader final
	{
		T Id {};
		uint32_t Size {};
	};
}