#include "memory.cc"
#include "processor.cc"

#include <systemc.h>

void ze_main() {
    Memory mem("Memory");         // Instantiate memory module
    Processor proc("Processor"); // Instantiate processor module

    proc.socket.bind(mem.socket); // Connect processor to memory

    sc_start(); // Start the simulation
}
int sc_main(int argc, char* argv[]) {
    ze_main();
    return 0;
}
