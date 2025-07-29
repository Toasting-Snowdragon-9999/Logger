// Logger.cpp
#include "logger.hpp"

std::unique_ptr<Logger> Logger::instance_ = nullptr;
std::once_flag Logger::init_flag_;

void Logger::initialize(std::ostream& stream, LogLevel level) {
    if (instance_ != nullptr) {
        std::cerr << "Logger already initialized" << std::endl;
    }
    
    std::call_once(init_flag_, [&]() {
        instance_ = std::make_unique<Logger>(stream, level, true);
    });
}

void Logger::initialize(const std::string& filename, LogLevel level) {
    if (instance_ != nullptr) {
        std::cerr << "Logger already initialized" << std::endl;
    }

    std::call_once(init_flag_, [&]() {
        instance_ = std::make_unique<Logger>(filename, level, false);
    });
}

Logger& Logger::instance__(){
    if (instance_ == nullptr)
        throw std::runtime_error("Logger not initialized. Call Logger::initialize().");
    return *instance_;
}

Logger::Logger(std::ostream& stream, LogLevel level, bool enable_colors) :
    out_(&stream), 
    min_level_(level),
    enable_colors_(enable_colors)
{

}

Logger::Logger(const std::string& filename, LogLevel level, bool enable_colors) :
    min_level_(level), 
    enable_colors_(enable_colors)
{
    auto fileStream = std::make_unique<std::ofstream>(filename, std::ios::app); //std::ios::out
    if (!fileStream->is_open()) {
        throw std::runtime_error("Failed to open log file: " + filename);
    }
    log_filename_ = filename; // Store the filename for future reference
    out_ = fileStream.get();       // set raw pointer for easy access
    owned_file_ = std::move(fileStream);  // take ownership
}

void Logger::log(LogLevel level, const std::string& msg, const std::source_location location){
    if (level < min_level_) return;

    const char* level_str = "";
    switch (level) {
        case LogLevel::TRACE: level_str = "[TRACE]"; break;
        case LogLevel::DEBUG: level_str = "[DEBUG]"; break;
        case LogLevel::INFO:  level_str = "[INFO ]"; break;
        case LogLevel::WARN:  level_str = "[WARN ]"; break;
        case LogLevel::ERROR: level_str = "[ERROR]"; break;
        case LogLevel::FATAL: level_str = "[FATAL]"; break;
    }
    if (file_settings_.enableRotation || owned_file_) {
        rotateLogFiles();
    }
    const char* filename = std::strrchr(location.file_name(), '/');
    if (!filename) {
        filename = std::strrchr(location.file_name(), '\\');
    }
    filename = filename ? filename + 1 : location.file_name();

    (*out_) << this->getColorCode(level) << level_str << this->getDefaultColorCode()
    << ": " << getTimestampString()
    << " [Thread: " << std::this_thread::get_id() << "]"  // Add thread ID
    << " " << filename << " - "
    << "`" << location.function_name() << "` "
    << '(' << location.line() << ':' << location.column() << ")"
    << " : " << msg << std::endl;
}

void Logger::logf(LogLevel level,
    std::format_string<Args...> format,
    std::source_location location = std::source_location::current(),
    Args&&... args)
{
    const std::string msg = std::format(format, std::forward<Args>(args)...);
    log(level, msg, location);
}

const char* Logger::getColorCode(LogLevel level){
    if (!enable_colors_)
        return ""; // No colors if useColors_ is false

    switch (level) {
        case LogLevel::TRACE: return "\033[36m"; // Cyan
        case LogLevel::DEBUG: return "\033[34m"; // Blue
        case LogLevel::INFO:  return "\033[37m"; // White
        case LogLevel::WARN:  return "\033[33m"; // Yellow
        case LogLevel::ERROR: return "\033[31m"; // Red
        case LogLevel::FATAL: return "\033[35m"; // Magenta
        default:              return "\033[0m";  // Reset
    }
}

const char* Logger::getDefaultColorCode(){
    if (!enable_colors_)
        return ""; // No colors if useColors_ is false
    return LoggerDefs::COLOR_RESET; // Reset color
}

std::string Logger::getTimestampString(){
    using namespace std::chrono;

    auto now = system_clock::now();
    auto now_time_t = system_clock::to_time_t(now);
    auto now_us = duration_cast<microseconds>(now.time_since_epoch()) % 1'000'000;

    std::tm local_tm;
    #if defined(_WIN32)
        localtime_s(&local_tm, &now_time_t);
    #else
        localtime_r(&now_time_t, &local_tm);
    #endif

    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%d-%m-%y %H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << now_us.count() / 1000;

    return oss.str();  // e.g., "25-07-25 14:33:12.123"
}

Logger::~Logger() {
    if (owned_file_) {
        owned_file_->flush();
        owned_file_->close();
    }
}

LogLevel Logger::getLogLevel() const {
    return min_level_;
}

void Logger::setLogLevel(LogLevel level) {
    min_level_ = level;
}

bool Logger::isColorEnabled() const {
    return enable_colors_;
}

bool Logger::configure(bool clear_file_on_startup, bool enable_rotation, int max_file_size, int max_backup_index) {
    file_settings_.clearFileOnStartup = clear_file_on_startup;
    file_settings_.enableRotation = enable_rotation;
    file_settings_.maxFileSize = max_file_size;
    file_settings_.maxBackupIndex = max_backup_index;
    return true;
}

bool Logger::configure(const LoggerDefs::FileSettings& settings) {
    file_settings_ = std::move(settings);
    return true;
}

std::uintmax_t getFileSize(const std::string& filename) {
    try {
        return std::filesystem::file_size(filename);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error getting file size: " << e.what() << '\n';
        return 0;
    }
}

std::string getNewFileName(std::string filename, int appended_idx){
    std::string new_filename = filename;
    size_t dot_pos = new_filename.find_last_of('.');
    if (dot_pos != std::string::npos) {
        new_filename.insert(dot_pos, "_" + std::to_string(appended_idx));
    } else {
        new_filename += "_" + std::to_string(appended_idx);
    }
    return new_filename;
}

bool Logger::rotateLogFiles() {
    owned_file_->flush();
    owned_file_->seekp(0, std::ios::end);

    if (owned_file_->tellp() < static_cast<std::streampos>(file_settings_.maxFileSize)) {
        return false; // No rotation needed
    }

    owned_file_->close();

    // Rotate existing files: log.txt.2 → log.txt.3, ..., log.txt.1 → log.txt.2
    for (int i = file_settings_.maxBackupIndex - 1; i >= 1; --i) {
        std::string old_name = getNewFileName(log_filename_, i - 1);  // e.g., log.txt.1
        std::string new_name = getNewFileName(log_filename_, i);      // e.g., log.txt.2

        if (std::filesystem::exists(old_name)) {
            std::filesystem::rename(old_name, new_name);
        }
    }

    // Rename current log file to .1
    std::string rotated_name = getNewFileName(log_filename_, 1);  // e.g., log.txt → log.txt.1
    std::filesystem::rename(log_filename_, rotated_name);

    // Open a new empty log file
    auto fileStream = std::make_unique<std::ofstream>(log_filename_, std::ios::trunc);
    if (!fileStream->is_open()) {
        throw std::runtime_error("Failed to open new log file: " + log_filename_);
    }

    out_ = fileStream.get();
    owned_file_ = std::move(fileStream);

    return true;
}

