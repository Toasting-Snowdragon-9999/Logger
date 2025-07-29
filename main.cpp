#include "logger.hpp"
#include "other_process.hpp"

int main(int argc, char* argv[]) {
    Logger::initialize(std::cout, LogLevel::TRACE);  // configure it once

    LoggerDefs::FileSettings settings = {
        .clearFileOnStartup = false,
        .enableRotation = false,
        .maxFileSize = 1024 * 1024, // 1 MB
        .maxBackupIndex = 5
    };
    //LOG_RCONFIGURE(false, true, 512, 5); // Clear file on startup, enable rotation, max size 1MB
    LOG_SCONFIGURE(settings); // Configure with settings
    LOG_TRACE("Starting application...");
    LOG_DEBUG("Initializing system with value: %d", 42);
    LOG_INFO("System ready");
    LOG_WARN("Low battery");
    LOG_ERROR("System failure");
    TEMP_TEST::run_this() ;  // call the function from another file
    TEMP_TEST::major_error();  // simulate a major error
    
    LOG_TRACE("Trying to log to file...");
    TEMP_TEST::OtherProcess otherProcess;
    otherProcess.run();  // run the other process logic
    return 0;
};
