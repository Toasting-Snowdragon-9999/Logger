#ifndef logger_defines_hpp
#define logger_defines_hpp

namespace LoggerDefs {
    
    struct FileSettings{
        bool clearFileOnStartup;    // Clear log file on startup
        bool enableRotation;        // Enable log rotation
        size_t maxFileSize;         // Maximum file size before rotation, in bytes
        int maxBackupIndex;         // Maximum number of backup files to keep
    };
    
    static constexpr const char* COLOR_RESET = "\033[0m";

};

namespace Logging {
  enum class Types {
    PARSER,
    NETWORK,
    FILE,
    SYSTEM,
    USER
  };

  enum class SubTypes {
    PARSER_ERROR,
    PARSER_WARNING,
    NETWORK_ERROR,
    NETWORK_WARNING,
    FILE_ERROR,
    FILE_WARNING,
    SYSTEM_ERROR,
    SYSTEM_WARNING,
    USER_ERROR,
    USER_WARNING
  };

  enum class User{
    CLIENT,
    DEVELOPER
  };


  
};



#endif // logger_defines_hpp