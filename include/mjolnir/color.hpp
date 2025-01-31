#ifndef MJOLNIR_COLOR_H
#define MJOLNIR_COLOR_H

#include <cstdint>      // for uint8_t, uint32_t
#include <string>       // for string
#include <string_view>  // for string_view

namespace mjolnir {
    class Color final {
        std::uint8_t r_{};
        std::uint8_t g_{};
        std::uint8_t b_{};

    public:
        constexpr Color() = default;

        explicit constexpr Color(std::uint32_t rgb) noexcept
            : r_{static_cast<std::uint8_t>((rgb >> 16) & 0xFF)}
            , g_{static_cast<std::uint8_t>((rgb >> 8) & 0xFF)}
            , b_{static_cast<std::uint8_t>(rgb & 0xFF)} {
        }

        constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept
            : r_{r}
            , g_{g}
            , b_{b} {
        }

        [[nodiscard]]
        std::string fg(std::string_view message) const;

        [[nodiscard]]
        std::string bg(std::string_view message) const;

        std::string fg_start() const;

        std::string bg_start() const;

        static constexpr std::string_view end{"\033[0m"};

        [[nodiscard]]
        constexpr std::uint8_t get_red() const noexcept {
            return r_;
        }

        [[nodiscard]]
        constexpr std::uint8_t get_green() const noexcept {
            return g_;
        }

        [[nodiscard]]
        constexpr std::uint8_t get_blue() const noexcept {
            return b_;
        }
    };

    namespace colors {
        static constexpr Color black{0x000000};
        static constexpr Color white{0xFFFFFF};
        static constexpr Color red{0xFF0000};
        static constexpr Color green{0x00FF00};
        static constexpr Color blue{0x0000FF};
        static constexpr Color yellow{0xFFFF00};
        static constexpr Color magenta{0xFF00FF};
        static constexpr Color cyan{0x00FFFF};
        static constexpr Color gray{0x808080};
        static constexpr Color dark_gray{0x404040};
        static constexpr Color light_gray{0xC0C0C0};
        static constexpr Color dark_red{0x800000};
        static constexpr Color dark_green{0x008000};
        static constexpr Color dark_blue{0x000080};
        static constexpr Color dark_yellow{0x808000};
        static constexpr Color dark_magenta{0x800080};
        static constexpr Color dark_cyan{0x008080};
        static constexpr Color light_red{0xFF8080};
        static constexpr Color light_green{0x80FF80};
        static constexpr Color light_blue{0x8080FF};
        static constexpr Color light_yellow{0xFFFF80};
        static constexpr Color light_magenta{0xFF80FF};
        static constexpr Color light_cyan{0x80FFFF};
    }// namespace colors
}// namespace mjolnir

#endif//MJOLNIR_COLOR_H
