#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <iostream>
#include <cstring>

SC_MODULE(Memory) {
    tlm_utils::simple_target_socket<Memory> socket; // Target socket for TLM communication
    static const unsigned int MEM_SIZE = 1024;      // 1 KB memory size
    uint8_t mem[MEM_SIZE];                          // Instruction memory array

    SC_CTOR(Memory) : socket("socket") {
        socket.register_b_transport(this, &Memory::b_transport);

        // Initialize memory with some instructions
        memset(mem, 0, MEM_SIZE); // Fill with zeros
#if 0
        mem[0] = 0x13;            // NOP (addi x0, x0, 0)
        mem[4] = 0x93;            // ADDI x1, x0, 0x1 (example instruction)
#endif
    }

    void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
        tlm::tlm_command cmd = trans.get_command();
        uint64_t addr = trans.get_address();
        uint8_t* data_ptr = trans.get_data_ptr();
        unsigned int len = trans.get_data_length();

        // Ensure the address is within range
        if (addr >= MEM_SIZE) {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }

        // Read or write based on transaction command
        if (cmd == tlm::TLM_READ_COMMAND) {
//            printf("Read at %x len %u - %x\n", (int)addr, len, (int)mem[addr]);
            memcpy(data_ptr, &mem[addr], len);
        } else if (cmd == tlm::TLM_WRITE_COMMAND) {
//            printf("Write at %x len %u - before=%x - data_ptr=%p - %x\n", (int)addr, len, (int)mem[addr], data_ptr, (int)data_ptr[addr]);
            memcpy(&mem[addr], data_ptr, len);
//            printf("Write at %x - after=%x - data_ptr=%p - %x\n", (int)addr, (int)mem[addr], data_ptr, (int)data_ptr[addr]);
        } else {
            throw std::runtime_error("Boom! unknown command");
        }

        trans.set_response_status(tlm::TLM_OK_RESPONSE); // Transaction successful
    }
};
