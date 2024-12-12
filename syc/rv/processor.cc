#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <iostream>

SC_MODULE(Processor) {
    tlm_utils::simple_initiator_socket<Processor> socket; // Initiator socket
    sc_core::sc_time clock_cycle;

    SC_CTOR(Processor) : socket("socket"), clock_cycle(10, sc_core::SC_NS) {
        SC_THREAD(fetch_instructions); // Process for instruction fetching
    }

    void fetch_instructions() {
        uint64_t pc = 0; // Program counter
        uint8_t buffer[4]; // Buffer to hold fetched instruction (4 bytes for RISC-V)
        tlm::tlm_generic_payload trans;
        sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

        for (int i = 0; i < 2; ++i) { // Fetch two instructions as an example
            trans.set_command(tlm::TLM_READ_COMMAND);
            trans.set_address(pc);
            trans.set_data_ptr(buffer);
            trans.set_data_length(4);
            trans.set_streaming_width(4);
            trans.set_byte_enable_ptr(0);
            trans.set_dmi_allowed(false);
            trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

            std::cout << "[Processor] Fetching instruction at PC: 0x" << std::hex << pc << std::endl;

            // Initiate transaction
            socket->b_transport(trans, delay);

            // Check response status
            if (trans.is_response_error()) {
                std::cerr << "[Processor] Error during instruction fetch!" << std::endl;
                break;
            }

            // Print fetched instruction
            std::cout << "[Processor] Instruction: 0x"
                      << std::hex << ((buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | buffer[0])
                      << std::endl;

            pc += 4; // Move to the next instruction (RISC-V instructions are 4 bytes)
            wait(clock_cycle); // Simulate delay for instruction fetch
        }
    }
};
