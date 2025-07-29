#ifndef LOGGER_CPP
#define LOGGER_CPP

//! Standard libs  
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <source_location>
#include <chrono>
#include <ctime>
#include <stdexcept>
#include <memory>
#include <mutex>
#include <filesystem>
#include <thread>
#include <format>

//! This header file defines the Logger macros.
#include "logger_defines.hpp"

#define LOG_TRACE(msg) do {                                  \
    Logger::instance__().log(LogLevel::TRACE, msg,       \
        std::source_location::current());                    \
} while(0)

#define LOG_DEBUG(msg, ...) do { \
    Logger::instance__().logf(LogLevel::DEBUG, msg, std::source_location::current(), ##__VA_ARGS__); \
} while(0)


#define LOG_INFO(msg) do {                                   \
    Logger::instance__().log(LogLevel::INFO, msg,        \
        std::source_location::current());                    \
} while(0)

#define LOG_WARN(msg) do {                                   \
    std::ostringstream oss; oss << msg;                      \
    Logger::instance__().log(LogLevel::WARN, oss.str(),        \
        std::source_location::current());                    \
} while(0)

#define LOG_ERROR(msg) do {                                  \
    std::ostringstream oss; oss << msg;                      \
    Logger::instance__().log(LogLevel::ERROR, oss.str(),       \
        std::source_location::current());                    \
} while(0)

#define LOG_FATAL(msg) do {                                  \
    std::ostringstream oss; oss << msg;                      \
    Logger::instance__().log(LogLevel::FATAL, oss.str(),       \
        std::source_location::current());                    \
} while(0)

#define LOG_RCONFIGURE(clearFileOnStartup, enableRotation, maxFileSize, maxBackupFiles) do { \
    Logger::instance__().configure(clearFileOnStartup, enableRotation, maxFileSize, maxBackupFiles); \
} while(0)

#define LOG_SCONFIGURE(settings) do { \
    Logger::instance__().configure(settings); \
} while(0)

//! Logger levels
enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

/**
 * @brief Logger class for logging messages with different severity levels.
 * Can log both to console and files, and supports colored output.
 * This class is a singleton, so it should be initialized once.
 */
class Logger 
{
public:

    /**
     * @brief Default constructor is deleted to prevent instantiation without parameters.
     * This is further to withhold the singleton pattern, and not allow multiple instances.
     */
    Logger() = delete; // Prevent default constructor

    /**
     * @brief Construct a new Logger object.
     * 
     * @param stream The strem from which to log messages.
     * @param level The minimum log level to log.
     */
    Logger(std::ostream& stream, LogLevel level, bool enable_colors);

    /**
     * @brief Construct a new Logger object for file logging.
     * 
     * @param filename The name of the file to log messages to.
     * @param level The minimum log level to log.
     * @throws std::runtime_error if the file cannot be opened.
     */
    Logger(const std::string& filename, LogLevel level, bool enable_colors);

    /**
     * @brief Destructor to clean up resources.
     */
    ~Logger();

    /**
     * @brief Initialize the Logger with a stream.
     * 
     * @param stream The output stream to log messages to (e.g., std::cout).
     * @param level The minimum log level to log messages.
     * @throws std::logic_error if the Logger is already initialized.
     */
    static void initialize(std::ostream& stream, LogLevel level);

    /**
     * @brief Initialize the Logger with a file.
     * 
     * @param filename The name of the file to log messages to.
     * @param level The minimum log level to log messages.
     * @throws std::runtime_error if the file cannot be opened.
     */
    static void initialize(const std::string& filename, LogLevel level);

    /**
     * @brief Get the singleton instance of the Logger.
     * 
     * @return Logger& Reference to the Logger instance.
     * @throws std::runtime_error if the Logger is not initialized.
     */
    static Logger& instance__();
    
    /**
     * @brief Configure the logger settings.
     * 
     * This function can be used to configure the logger settings such as
     * clearing the log file on startup, enabling rotation, and setting the
     * maximum file size. Only use when loggin to file.
     * 
     * @param clearFileOnStartup If true, clears the log file on startup.
     * @param enableRotation If true, enables log rotation.
     * @param maxFileSize Maximum size of the log file before rotation.
     * @return true if configuration was successful, false otherwise.
     */
    bool configure(bool clear_file_on_startup = false, bool enable_rotation = false, int max_file_size = 0, int max_backup_index = 0);

    /**
     * @brief Configure the logger settings.
     * 
     * This function can be used to configure the logger settings such as
     * clearing the log file on startup, enabling rotation, and setting the
     * maximum file size. Only use when loggin to file.
     * 
     * @param settings The settings to configure the logger with.
     * @return true if configuration was successful, false otherwise.
     */
    bool configure(const LoggerDefs::FileSettings& settings);

    /**
     * @brief Log a message with a specific log level.
     * 
     * @param level The log level of the message.
     * @param msg The message to log.
     * @param location The source location of the log call.
     */
    void log(LogLevel level, const std::string& msg,
        const std::source_location location = std::source_location::current());

    /**
     * @brief Log a formatted message with a specific log level.
     * @tparam Args The types of the arguments to format.
     * @param level The log level of the message.
     * @param format The format string for the message.
     * @param args The arguments to format into the message.
     * @note This function uses std::format to format the message.
     * @throws std::format_error if the format string is invalid.
     */
    template<typename... Args>
    void logf(LogLevel level,
              std::format_string<Args...> format,
              std::source_location location = std::source_location::current(),
              Args&&... args) {...}
    
    /**
     * @brief Get the minimum log level.
     * 
     * @return LogLevel The minimum log level.
     */
    LogLevel getLogLevel() const;

    /**
     * @brief Set the minimum log level.
     * 
     * @param level The new minimum log level.
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief Check if colored output is enabled.
     * 
     * @return true if colored output is enabled, false otherwise.
     */
    bool isColorEnabled() const;

private:

    /**
     * @brief Get the color code for a specific log level.
     * 
     * @param level The log level to get the color code for.
     * @return const char* The color code as a string.
     */
    const char* getColorCode(LogLevel level);

    /**
     * @brief Get the default color code.
     * 
     * @return const char* The default color code as a string.
     */
    const char* getDefaultColorCode();

    /**
     * @brief Get the current timestamp as a string.
     * 
     * @return std::string The current timestamp in the format "dd-mm-yy HH:MM:SS.mmm".
     */
    std::string getTimestampString();

    /**
     * @brief Rotate log files if necessary.
     * This function checks the current log file size and rotates the log files if
     * the size exceeds the maximum file size configured.
     * 
     * @return true if rotation was successful, false otherwise.
     * This function is only relevant when logging to a file.
     */
    bool rotateLogFiles();

private: 
    std::ostream* out_;                             //! Pointer to the output stream, can be console or file
    LogLevel min_level_;                            //! Minimum log level to log messages  
    std::unique_ptr<std::ofstream> owned_file_;     //! Unique pointer to the file stream if logging to a file
    static std::unique_ptr<Logger> instance_;       //! Static instance of the Logger class, used for singleton pattern
    bool enable_colors_;                            //! Flag to indicate if colored output is allowed
    static std::once_flag init_flag_;               //! Flag to ensure Logger is initialized only once
    LoggerDefs::FileSettings file_settings_;        //! File settings for logging to file
    std::string log_filename_;                     //! Name of the log file if logging to a file
};

#endif // LOGGER_CPP