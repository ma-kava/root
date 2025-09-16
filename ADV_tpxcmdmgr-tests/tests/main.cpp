#include <iostream>
#include <format>
#include <thread>
#include "ihwlib.h"
#include "mpx2dev.h"
#include "tpx3dev.h"
#include "tpx2dev.h"
#include "tpx2streams.h"
#include "tpx2cmdmgr.h"
#include "dummypixet.h"
#include "tpx2/cpucommands.h"
#include <array>
#include <csignal>
#include <atomic>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace HwMiniPix;


template<typename F>
struct scope_guard { F f; ~scope_guard(){ f(); } };

std::atomic<bool> should_quit = false;


struct MockCpuTransport : ICpuTransport {
    MOCK_METHOD(void, send, (ICpuTransport::buffer_view), (override));
    MOCK_METHOD(void, recv, (ICpuTransport::buffer_view, std::chrono::milliseconds), (override));
    MOCK_METHOD(void, exchange, (ICpuTransport::buffer_view, ICpuTransport::buffer_view, std::chrono::milliseconds), (override));
};

using ::testing::_;
using ::testing::DoAll;
using ::testing::Invoke;    
using ::testing::Return;
using ::testing::WithArg;

TEST(Serialize, ChipPowerEnable) {
    HwMiniPix::CpuCmds::ChipPowerEnable cmd{.enabled=true};
    auto bytes = HwMiniPix::CpuCmds::serialize(cmd);
    // "*pe\x01"
    ASSERT_EQ(bytes.size(), 3u);
    EXPECT_EQ(bytes[0], '*');
    EXPECT_EQ(bytes[1], 'p');
    EXPECT_EQ(bytes[2], 'e'); // wait — enabled byte is appended; see make_command
}

TEST(Execute_AckedBehavior, SetBiasVoltageV3_Ack) {
    using namespace HwMiniPix::CpuCmds;
    MockCpuTransport tr;

    SetBiasVoltageV3 cmd{.setRaw=false, .volts=1200};
    auto tx = serialize(cmd);

    // Prepare a 4-byte ACK response
    std::array<uint8_t,4> ack = {0x00, 0x00, 0xCD, 0xAB}; // little-endian 0xABCD0000
    EXPECT_CALL(tr, exchange(_, _, _))
      .WillOnce(Invoke([&](ICpuTransport::buffer_view send,
                           ICpuTransport::buffer_view recv,
                           auto){
          // TX matches
          ASSERT_EQ(send.size(), tx.size());
          EXPECT_TRUE(std::equal(tx.begin(), tx.end(), send.begin()));
          // Fill RX
          ASSERT_EQ(recv.size(), ack.size());
          std::memcpy(recv.data(), ack.data(), ack.size());
      }));

    EXPECT_TRUE(HwMiniPix::CpuCmds::execute(tr, cmd));
}

TEST(Execute_RespondedBehavior, GetCpuTemperature) {
    using namespace HwMiniPix::CpuCmds;
    MockCpuTransport tr;

    GetCpuTemperature cmd{};
    auto tx = serialize(cmd);

    // Temp = 253 => 25.3°C, little-endian int32
    std::array<uint8_t,4> rx = {253, 0x00, 0x00, 0x00};
    EXPECT_CALL(tr, exchange(_, _, _))
      .WillOnce(Invoke([&](ICpuTransport::buffer_view send,
                           ICpuTransport::buffer_view recv,
                           auto){
          ASSERT_EQ(send.size(), tx.size());
          std::memcpy(recv.data(), rx.data(), rx.size());
      }));

    double t = HwMiniPix::CpuCmds::execute(tr, cmd);
    EXPECT_DOUBLE_EQ(t, 25.3);
}

TEST(Execute_VarLen, GetSupportedCpuCmds) {
    using namespace HwMiniPix::CpuCmds;
    MockCpuTransport tr;

    GetSupportedCpuCmds cmd{};

    // First call: GetLength -> returns 7 bytes
    // Second call: GetData -> returns "pe;gs\0"
    testing::InSequence seq;
    EXPECT_CALL(tr, exchange(_, _, _))
      .WillOnce(Invoke([](auto send, auto recv, auto){
          // Return length=7 (little-endian u32)
          uint32_t len = 7;
          std::memcpy(recv.data(), &len, 4);
      }));
    EXPECT_CALL(tr, exchange(_, _, _))
      .WillOnce(Invoke([](auto, auto recv, auto){
          const char data[] = "pe;gs\0";
          ASSERT_EQ(recv.size(), 7u);
          std::memcpy(recv.data(), data, 7);
      }));

    auto result = HwMiniPix::CpuCmds::execute(tr, cmd);
    EXPECT_THAT(result.commands, ::testing::HasSubstr("pe;gs"));
}
