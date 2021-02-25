#pragma once

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