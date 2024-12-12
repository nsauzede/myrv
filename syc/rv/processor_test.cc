#include <gtest/gtest.h>

#include "processor.cc"

class ProcessorTest : public ::testing::Test {
};

TEST_F(ProcessorTest, InstructionFetch) {
#if 0
    // Mock memory to return specific instructions
    MockMemory mem;
    proc->socket.bind(mem.socket);

    EXPECT_CALL(mem, b_transport(_))
        .Times(2)
        .WillRepeatedly([](tlm::tlm_generic_payload& trans, sc_time&) {
            uint8_t* data_ptr = trans.get_data_ptr();
            data_ptr[0] = 0x13; // NOP
            data_ptr[1] = 0x93; // ADDI
        });

    proc->fetch_instructions();
    // Check output/logs for fetched instructions
#endif
}
