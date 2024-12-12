#include "memory.cc"

#include <ut/ut.h>

TESTCASE(TestMemory)
class MemoryTest {
public:
    Memory* mem; // Pointer to the Memory module
    tlm::tlm_generic_payload trans;
    sc_core::sc_time delay;

    void SetUp() {
        mem = new Memory("Memory");
        delay = sc_core::sc_time(0, sc_core::SC_NS);
    }

    void TearDown() {
        delete mem;
    }
};
    TESTMETHOD(test_Initialization) {
        MemoryTest m;
        m.SetUp();

        // Test that memory is initialized to 0
        uint8_t buffer[4];
        m.trans.set_command(tlm::TLM_READ_COMMAND);
        m.trans.set_address(0);
        m.trans.set_data_ptr(buffer);
        m.trans.set_data_length(4);

        m.mem->b_transport(m.trans, m.delay);
        EXPECT_EQ(buffer[0], 0);
        EXPECT_EQ(buffer[1], 0);
        EXPECT_EQ(buffer[2], 0);
        EXPECT_EQ(buffer[3], 0);

        m.TearDown();
    }
    TESTMETHOD(test_ReadWrite) {
        MemoryTest m;
        m.SetUp();

        // Test writing and reading back from memory
        uint8_t write_data[4] = {0xDE, 0xAD, 0xBE, 0xEF};
        uint8_t read_data[4] = {0};

        // Write data
        m.trans.set_command(tlm::TLM_WRITE_COMMAND);
        m.trans.set_address(4);
        m.trans.set_data_ptr(write_data);
        m.trans.set_data_length(sizeof(write_data));
        m.mem->b_transport(m.trans, m.delay);
        EXPECT_EQ(m.trans.get_response_status(), tlm::TLM_OK_RESPONSE);

        // Read data back
        m.trans.set_command(tlm::TLM_READ_COMMAND);
        m.trans.set_data_ptr(read_data);
        m.mem->b_transport(m.trans, m.delay);
        EXPECT_EQ(m.trans.get_response_status(), tlm::TLM_OK_RESPONSE);

        EXPECT_EQ(read_data[0], 0xDE);
        EXPECT_EQ(read_data[1], 0xAD);
        EXPECT_EQ(read_data[2], 0xBE);
        EXPECT_EQ(read_data[3], 0xEF);

        m.TearDown();
    }
    TESTMETHOD(test_OutOfBounds) {
        MemoryTest m;
        m.SetUp();

        // Test accessing an out-of-bounds address
        uint8_t buffer[4];
        m.trans.set_command(tlm::TLM_READ_COMMAND);
        m.trans.set_address(1024); // Out of bounds
        m.trans.set_data_ptr(buffer);
        m.mem->b_transport(m.trans, m.delay);

        EXPECT_EQ(m.trans.get_response_status(), tlm::TLM_ADDRESS_ERROR_RESPONSE);

        m.TearDown();
    }
