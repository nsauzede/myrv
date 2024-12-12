#include <systemc.h>

// Define a module
SC_MODULE(HelloWorld) {
    // Constructor
    SC_CTOR(HelloWorld) {
        // Process registration
        SC_METHOD(say_hello);
    }

    // Process definition
    void say_hello() {
        std::cout << "Hello, SystemC!" << std::endl;
    }
};

void ze_main() {
    HelloWorld hello("HelloModule"); // Instantiate the module
    sc_start();                      // Start simulation
}

// Main function
int sc_main(int argc, char* argv[]) {
    ze_main();
    return 0;
}
