#pragma once

#include <span>
#include <array>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <tuple>
#include <chrono>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "tpx2cmdmgr.h"
#include "tpx2/cpucommands.h"

template <size_t N = std::dynamic_extent>
using buff_view = std::span<uint8_t, N>;

using namespace HwMiniPix;

class MockCpuTransport : public ICpuTransport {
public:
    MOCK_METHOD(void, send, (ICpuTransport::buffer_view), (override));
    MOCK_METHOD(void, recv, (ICpuTransport::buffer_view, std::chrono::milliseconds), (override));
    MOCK_METHOD(void, exchange, (ICpuTransport::buffer_view, ICpuTransport::buffer_view, std::chrono::milliseconds), (override));
};

namespace detail {
    template<typename T>
    constexpr std::size_t arg_size() {
        if constexpr (std::is_arithmetic_v<T>) {
            static_assert(sizeof(T) == 1, "Scalar arguments must be single bytes");
            return 1;
        } else {
            return std::tuple_size_v<std::remove_cvref_t<T>>;
        }
    }
}

template <std::size_t N, typename... Args>
static void testCommandPrefixAndArgs(const std::array<std::uint8_t, N>& bytes, const char* mnemonic, Args... args)
{
    ASSERT_EQ(bytes[0], '*') << "All commands expected to begin with '*'";
    size_t mlen = std::strlen(mnemonic);
    size_t argslen = (detail::arg_size<Args>() + ... + 0);
    ASSERT_EQ(bytes.size(), 1 + mlen + argslen);

    for (size_t i = 0; i < mlen; ++i) {
        EXPECT_EQ(bytes[1 + i], static_cast<std::uint8_t>(mnemonic[i]))
            << "Mnemonic mismatch at index " << i;
    }
}

// --------------------- Serialize macros ---------------------

#define DEFINE_SERIALIZE_TEST_POSTED(CommandType, Mnemonics) \
TEST(Serialize_PostedBehavior, CommandType) { \
    HwMiniPix::CpuCmds::CommandType cmd{}; \
    auto bytes = HwMiniPix::CpuCmds::serialize(cmd); \
    testCommandPrefixAndArgs(bytes, Mnemonics); \
}

#define DEFINE_SERIALIZE_TEST_ACKED(CommandType, Mnemonics, ...) \
TEST(Serialize_AckedBehavior, CommandType) { \
    HwMiniPix::CpuCmds::CommandType cmd{}; \
    auto expectedBytes = HwMiniPix::CpuCmds::serialize(cmd); \
    testCommandPrefixAndArgs(expectedBytes, Mnemonics, ##__VA_ARGS__); \
}

#define DEFINE_SERIALIZE_TEST_RESPONDED_INIT(CommandType, InitExpr, Mnemonics, ...) \
TEST(Serialize_RespondedBehavior, CommandType) { \
    HwMiniPix::CpuCmds::CommandType cmd = InitExpr; \
    auto bytes = HwMiniPix::CpuCmds::serialize(cmd); \
    testCommandPrefixAndArgs(bytes, Mnemonics, ##__VA_ARGS__); \
}

#define DEFINE_SERIALIZE_TEST_RESPONDED(CommandType, Mnemonics, ...) \
TEST(Serialize_RespondedBehavior, CommandType) { \
    HwMiniPix::CpuCmds::CommandType cmd{}; \
    auto bytes = HwMiniPix::CpuCmds::serialize(cmd); \
    testCommandPrefixAndArgs(bytes, Mnemonics, ##__VA_ARGS__); \
}

// ---------------------- Execute macros ----------------------
#define DEFINE_EXECUTE_TEST_POSTED(CommandType) \
TEST(Execute_PostedBehavior, CommandType) { \
    MockCpuTransport mockTransport; \
    HwMiniPix::CpuCmds::CommandType cmd{}; \
    auto expectedSend = HwMiniPix::CpuCmds::serialize(cmd); \
    EXPECT_CALL(mockTransport, send(_)) \
        .Times(1) \
        .WillOnce(Invoke([&](ICpuTransport::buffer_view send_bv) { \
            ASSERT_EQ(send_bv.size(), expectedSend.size()); \
            for (size_t i = 0; i < expectedSend.size(); ++i) \
                EXPECT_EQ(send_bv[i], expectedSend[i]); \
        })); \
    HwMiniPix::CpuCmds::execute(mockTransport, cmd); \
}

#define DEFINE_EXECUTE_TEST_ACKED_BOOLRESULT(CommandType, BuildCmdExpr) \
TEST(Execute_AckedBehavior, CommandType##AckThenNack) { \
    MockCpuTransport mockTransport; \
    auto cmd = BuildCmdExpr; \
    auto expectedSend = HwMiniPix::CpuCmds::serialize(cmd); \
    EXPECT_CALL(mockTransport, exchange(_, _, _)) \
        .WillOnce(Invoke([&](ICpuTransport::buffer_view sb, \
                             ICpuTransport::buffer_view rb, \
                             std::chrono::milliseconds) { \
            rb[0] = 0x00; rb[1] = 0x00; rb[2] = 0xCD; rb[3] = 0xAB; \
        })); \
    bool ok = HwMiniPix::CpuCmds::execute(mockTransport, cmd); \
    EXPECT_TRUE(ok); \
    EXPECT_CALL(mockTransport, exchange(_, _, _)) \
        .WillOnce(Invoke([&](ICpuTransport::buffer_view, \
                             ICpuTransport::buffer_view rb, \
                             std::chrono::milliseconds) { \
            rb[0] = 0x01; rb[1] = 0x00; rb[2] = 0xCD; rb[3] = 0xAB; \
        })); \
    EXPECT_THROW(HwMiniPix::CpuCmds::execute(mockTransport, cmd), std::exception); \
}

// ---------------------- Endian helpers ----------------------

inline constexpr auto to_bytes_be(uint32_t val) {
    return std::array<uint8_t, 4>{
        static_cast<uint8_t>(val >> 24),
        static_cast<uint8_t>(val >> 16),
        static_cast<uint8_t>(val >> 8),
        static_cast<uint8_t>(val)
    };
}

inline constexpr auto to_bytes_be(uint16_t val) {
    return std::array<uint8_t, 2>{
        static_cast<uint8_t>(val >> 8),
        static_cast<uint8_t>(val)
    };
}

inline constexpr auto to_bytes_le(uint32_t val) {
    return std::array<uint8_t, 4>{
        static_cast<uint8_t>(val),
        static_cast<uint8_t>(val >> 8),
        static_cast<uint8_t>(val >> 16),
        static_cast<uint8_t>(val >> 24)
    };
}

inline constexpr auto to_bytes_le(uint16_t val) {
    return std::array<uint8_t, 2>{
        static_cast<uint8_t>(val),
        static_cast<uint8_t>(val >> 8)
    };
}

inline constexpr auto to_bytes_le(int16_t val) {
    return std::array<uint8_t, 2>{
        static_cast<uint8_t>(val),
        static_cast<uint8_t>(val >> 8)
    };
}

inline constexpr auto to_bytes_le(uint8_t val) {
    return std::array<uint8_t, 2>{
        static_cast<uint8_t>(val), 
        static_cast<uint8_t>(val >> 8)};
}

inline constexpr auto to_bytes_le(bool val) {
    return std::array<uint8_t, 1>{ static_cast<uint8_t>(!!val) };
}

// ---------------------- Common aliases ----------------------
namespace HwMiniPix { namespace CpuCmds {
    using GetParamsLen_HK = GetParamsLen<HwMiniPix::ParamsLen::HOUSE_KEEPING_DATA>;
}}
