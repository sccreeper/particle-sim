#include "logging.hpp"

namespace logging
{
    
    void message(const std::string &msg, Level lev) {

        std::string infoString;

        switch (lev)
        {
        case Info:
            infoString = "Info";
            break;
        case Warning:
            infoString = "Warning";
            break;
        case Debug:
            infoString = "Debug";
            break;
        }

        std::cout << infoString << ": " << msg << std::endl;

    }

} // namespace logging
