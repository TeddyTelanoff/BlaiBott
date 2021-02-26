#pragma once
// Define defined(SY_NET_ENABLE_LOGGING) to enable server and client logging to std::cout & std::cerr
// Define defined(SY_NET_SHOW_VALIDATION) to enable server and client validation key logging to std::cout
// Define defined(SY_NET_DISABLE_EXCEPTION_LOGGING) to disable server and client exceptions logging to std::cerr

#include <queue>
#include <memory>
#include <cstdint>
#include <iostream>
#include <algorithm>

#if defined(_WIN32)
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

namespace Symple::Net
{
	using ScrambleFunction = uint64_t(*)(uint64_t);

	uint64_t Scramble(uint64_t in)
	{
		uint64_t out = in ^ 0x8BEEFFa;
		out = (out & 0x694201337) >> 4 | (out & 0x133742069) << 4;
		return out ^ 0xC0DE2FACE;
	}
}