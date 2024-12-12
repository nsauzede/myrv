#include "memory.cc"

#include <gtest/gtest.h>

class MemoryTest : public ::testing::Test {
protected:
    Memory* mem; // Pointer to the Memory module
    tlm::tlm_generic_payload trans;
    sc_core::sc_time delay;

    void SetUp() override {
        mem = new Memory("Memory");
        delay = sc_core::sc_time(0, sc_core::SC_NS);
    }

    void TearDown() override {
        delete mem;
    }
};

TEST_F(MemoryTest, Initialization) {
    // Test that memory is initialized to 0
    uint8_t buffer[4];
    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_address(0);
    trans.set_data_ptr(buffer);
    trans.set_data_length(4);

    mem->b_transport(trans, delay);
    EXPECT_EQ(buffer[0], 0);
    EXPECT_EQ(buffer[1], 0);
    EXPECT_EQ(buffer[2], 0);
    EXPECT_EQ(buffer[3], 0);
}

TEST_F(MemoryTest, ReadWrite) {
    // Test writing and reading back from memory
    uint8_t write_data[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t read_data[4] = {0};

    // Write data
    trans.set_command(tlm::TLM_WRITE_COMMAND);
    trans.set_address(4);
    trans.set_data_ptr(write_data);
    trans.set_data_length(sizeof(write_data));
    mem->b_transport(trans, delay);
    EXPECT_EQ(trans.get_response_status(), tlm::TLM_OK_RESPONSE);

    // Read data back
    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_data_ptr(read_data);
    mem->b_transport(trans, delay);
    EXPECT_EQ(trans.get_response_status(), tlm::TLM_OK_RESPONSE);

    EXPECT_EQ(read_data[0], 0xDE);
    EXPECT_EQ(read_data[1], 0xAD);
    EXPECT_EQ(read_data[2], 0xBE);
    EXPECT_EQ(read_data[3], 0xEF);
}

TEST_F(MemoryTest, OutOfBounds) {
    // Test accessing an out-of-bounds address
    uint8_t buffer[4];
    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_address(1024); // Out of bounds
    trans.set_data_ptr(buffer);
    mem->b_transport(trans, delay);

    EXPECT_EQ(trans.get_response_status(), tlm::TLM_ADDRESS_ERROR_RESPONSE);
}
