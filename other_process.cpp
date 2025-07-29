#include "other_process.hpp"

void TEMP_TEST::run_this() {
    LOG_DEBUG("From a new file"); // Use the service
}

void TEMP_TEST::major_error() {
    LOG_FATAL("Major error occurred in other_process.cpp");
}

void test_function() {
    LOG_INFO("Thread started in OtherProcess");
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        LOG_DEBUG("Thread iteration: %d " , i);
    }
}


void new_thread(){
    std::thread t(test_function);

    t.join(); // Wait for the thread to finish
}


void TEMP_TEST::OtherProcess::run() {
    LOG_INFO("Running OtherProcess: " + name_);
    new_thread(); // Call the new thread function
}
