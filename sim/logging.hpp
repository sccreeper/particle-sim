#pragma once
#include <string>

namespace logging {

    enum Level { Info, Warning, Debug };

    void message(const std::string &msg, Level lev = Info);
} // namespace logging
