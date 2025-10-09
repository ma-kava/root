#include "main.hpp"
// #include "cpucommands.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::WithArg;

TEST(Parse_AckedBehaviod, ChipPowerEnable) 
{
    std::array<uint8_t, 4> bytes1 = {0x78, 0x56, 0x34, 0x12}; // little-endian 0x12345678
    uint32_t result = HwMiniPix::CpuCmds::parse_uint32_le(buff_view<4>(bytes1));
    EXPECT_EQ(result, 0x12345678u);

    // edge cases
    std::array<uint8_t, 4> bytes2 = {0x01, 0x00, 0x00, 0x00};
    EXPECT_EQ(HwMiniPix::CpuCmds::parse_uint32_le(buff_view<4>(bytes2)), 1u);

    std::array<uint8_t, 4> bytes3 = {0xFF, 0xFF, 0xFF, 0xFF};
    EXPECT_EQ(HwMiniPix::CpuCmds::parse_uint32_le(buff_view<4>(bytes3)), 0xFFFFFFFFu);

    std::array<uint8_t, 4> bytes4 = {0x00, 0x00, 0x00, 0x00};
    EXPECT_EQ(HwMiniPix::CpuCmds::parse_uint32_le(buff_view<4>(bytes4)), 0u);
}

TEST(Execute_RespondedBehavior, ReadCpuFirmwareVersion)
{
    MockCpuTransport mockTransport;
    HwMiniPix::CpuCmds::ReadCpuFirmwareVersion cmd{};
    auto expectedSend = HwMiniPix::CpuCmds::serialize(cmd);

    EXPECT_CALL(mockTransport, exchange(_, _, _))
        .Times(1)
        .WillOnce(Invoke([&](ICpuTransport::buffer_view send_bv, ICpuTransport::buffer_view recv_bv, std::chrono::milliseconds) {
            // 1. Validate send buffer contents (do this only once)
            ASSERT_EQ(send_bv.size(), expectedSend.size());
            for (size_t i = 0; i < expectedSend.size(); ++i)
                EXPECT_EQ(send_bv[i], expectedSend[i]);

            // 2. Simulate MCU response: fill recv_bv with expected protocol values
            recv_bv[0] = 'M'; recv_bv[1] = 'P'; recv_bv[2] = 'T'; recv_bv[3] = '2';
            recv_bv[4] = (2u << 4) | 1u; // major=2, minor=1
            recv_bv[5] = 2u;   // day
            recv_bv[6] = 10u;  // month
            recv_bv[7] = 24u;  // year
        }));

    auto result = HwMiniPix::CpuCmds::execute(mockTransport, cmd);

    // 3. Assert that the parser produced the correct FirmwareVersion struct
    EXPECT_EQ(result.major, 2u);
    EXPECT_EQ(result.minor, 1u);
    EXPECT_EQ(result.build.day, 2u);
    EXPECT_EQ(result.build.month, 10u);
    EXPECT_EQ(result.build.year, 24u);
}

TEST(Parse_AckedBehaviod, ReadCpuFirmwareVersionThrowsOnBadMagic)
{
    HwMiniPix::CpuCmds::ReadCpuFirmwareVersion cmd{};

    // Corrupt the magic (first byte), keep the rest valid
    std::array<uint8_t, 8> recv{
        'X','P','T','2',          // wrong magic
        static_cast<uint8_t>((2u << 4) | 1u),
        2u, 10u, 24u
    };

    EXPECT_THROW({
        (void)HwMiniPix::CpuCmds::parse(cmd, buff_view<8>(recv));
    }, IntException);
}

// ---------------------- Serialize tests using macros ----------------------

// Key / system
DEFINE_SERIALIZE_TEST_RESPONDED(CheckCpuFirmwareKey, "k")
DEFINE_SERIALIZE_TEST_RESPONDED(GetCpuStatus, "gs")
DEFINE_SERIALIZE_TEST_RESPONDED(GetCpuStatusText, "gt")
DEFINE_SERIALIZE_TEST_RESPONDED(IsLastRequestFinished, "if")
DEFINE_SERIALIZE_TEST_RESPONDED(ReadMotorHours, "mr")
DEFINE_SERIALIZE_TEST_RESPONDED_INIT(GetParamsLen_HK, { .param = HwMiniPix::ParamsLen::HOUSE_KEEPING_DATA }, "gl", 'h', 'k')

// Measurements
DEFINE_SERIALIZE_TEST_RESPONDED(GetCpuTemperature, "t")
DEFINE_SERIALIZE_TEST_RESPONDED(GetCurrent, "C", 'i')
DEFINE_SERIALIZE_TEST_RESPONDED(GetPowerSupplyVoltage, "V", 'C')
DEFINE_SERIALIZE_TEST_RESPONDED(GetADC, "a", static_cast<uint8_t>(0))

// External flash
DEFINE_SERIALIZE_TEST_RESPONDED(IsExtFlashConnected, "xi")
DEFINE_SERIALIZE_TEST_ACKED(UnlockExtFlash, "xu")
DEFINE_SERIALIZE_TEST_RESPONDED(ExtFlashStatus, "xs")
DEFINE_SERIALIZE_TEST_ACKED(
    EraseExtFlashBlock, "xe", 
    static_cast<uint8_t>((1 >> 16) & 0xFF),
    static_cast<uint8_t>((1 >> 8) & 0xFF), 
    static_cast<uint8_t>(1 & 0xFF)
)
DEFINE_SERIALIZE_TEST_RESPONDED(ReadFlashParameters, "r", to_bytes_be(uint32_t{0}), to_bytes_be(static_cast<uint32_t>(496 / 4)))

// Bias and DAC
DEFINE_SERIALIZE_TEST_ACKED(SetBiasDacVoltage, "d", to_bytes_be(static_cast<uint16_t>(2048.0 / 2.04 * 2.3 + 0.5)))
DEFINE_SERIALIZE_TEST_ACKED(SetBiasVoltageV3, "Bs", to_bytes_le(bool{false}), to_bytes_be(static_cast<uint16_t>(2)))
DEFINE_SERIALIZE_TEST_RESPONDED(GetBiasV3, "Bg", to_bytes_le(static_cast<uint8_t>(0)))
DEFINE_SERIALIZE_TEST_ACKED(SetDacNew, "d", static_cast<uint8_t>(2), to_bytes_le(static_cast<uint32_t>(23)))

// ----------------------- Execute tests using macros -----------------------

DEFINE_EXECUTE_TEST_ACKED_BOOLRESULT(ChipPowerEnable, HwMiniPix::CpuCmds::ChipPowerEnable{ .enabled = true })
DEFINE_EXECUTE_TEST_ACKED_BOOLRESULT(SetBiasVoltageV3, HwMiniPix::CpuCmds::SetBiasVoltageV3{ .volts = 3 })
DEFINE_EXECUTE_TEST_ACKED_BOOLRESULT(SetDacNew, HwMiniPix::CpuCmds::SetDacNew{ .chan = 8 })
DEFINE_EXECUTE_TEST_POSTED(StartMainFirmware)
DEFINE_EXECUTE_TEST_POSTED(SetBiasDacVoltage)
