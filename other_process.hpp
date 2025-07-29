#ifndef OTHER_PROCESS_CPP
#define OTHER_PROCESS_CPP

#include "logger.hpp"
#include <iostream>
#include <string>
#include <thread>

namespace TEMP_TEST{
    void run_this();

    void major_error();

    class OtherProcess {
    public:
        OtherProcess() = default; // Default constructor
        void run();
        
    private:
        std::string name_ = "OtherProcess";
        // Private members can be added here if needed
    };
} // namespace

#endif