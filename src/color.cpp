#include <format>           // for format
#include <mjolnir/color.hpp>// for Color
#include <sstream>          // for char_traits, basic_ostream, basic_strin...
#include <string>           // for allocator, operator<<, string
#include <string_view>      // for operator<<, string_view

namespace mjolnir {
    std::string Color::fg(std::string_view message) const {
        std::stringstream ss;

        ss << fg_start() << message << end;
        return ss.str();
    }

    std::string Color::bg(std::string_view message) const {
        std::stringstream ss;

        ss << bg_start() << message << end;
        return ss.str();
    }

    std::string Color::fg_start() const {
        return std::format(
                "\033[38;2;{};{};{}m", static_cast<int>(r_),
                static_cast<int>(g_), static_cast<int>(b_)
        );
    }

    std::string Color::bg_start() const {
        return std::format(
                "\033[48;2;{};{};{}m", static_cast<int>(r_),
                static_cast<int>(g_), static_cast<int>(b_)
        );
    }
}// namespace mjolnir
