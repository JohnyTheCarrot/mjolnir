#ifndef MJOLNIR_SOURCE_H
#define MJOLNIR_SOURCE_H

#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "color.hpp"
#include "span.hpp"

namespace mjolnir {
    struct LabelDisplay final {
        std::optional<std::string> message_;
        std::optional<Color>       color_{};

        void print(std::ostream &os, std::string_view message) const;
    };

    class Label final {
        Span         span_;
        LabelDisplay display_{};

    public:
        explicit Label(Span const &span);

        [[nodiscard]]
        Span const &get_span() const noexcept;

        [[nodiscard]]
        LabelDisplay const &get_display() const noexcept;

        Label &with_message(std::string message);

        Label &with_color(Color color);

        [[nodiscard]]
        Span get_subspan(Span const &span) const noexcept;
    };

    struct Line final {
        std::size_t byte_offset_;
        std::size_t byte_length_;
        std::size_t line_number_;

        [[nodiscard]]
        bool operator==(Line const &other) const;

        [[nodiscard]]
        bool operator<(Line const &other) const;

        [[nodiscard]]
        std::size_t end() const noexcept;

        [[nodiscard]]
        Span get_subspan(Span const &span) const noexcept;

        [[nodiscard]]
        std::size_t get_column(std::size_t offset) const noexcept;
    };

    namespace internal {
        struct SpannedLine final {
            Line                  line_;
            std::set<ColoredSpan> spans_;

            [[nodiscard]]
            bool operator==(SpannedLine const &other) const;

            [[nodiscard]]
            bool operator<(SpannedLine const &other) const;

            [[nodiscard]]
            bool has_highlightable_span() const noexcept;

            [[nodiscard]]
            std::size_t max_span_end() const;
        };
    }// namespace internal

    class Source final {
        std::string       name_;
        std::string       buffer_;
        std::vector<Line> lines_;

    public:
        Source(std::string name, std::string buffer);

        [[nodiscard]]
        std::string_view get_name() const noexcept;

        [[nodiscard]]
        std::optional<std::string_view> get_line(std::size_t offset) const;

        [[nodiscard]]
        std::string_view get_line(Line const &line) const;

        [[nodiscard]]
        std::string_view get_line(Line const &line, Span const &span) const;

        [[nodiscard]]
        std::optional<Line> get_line_info(std::size_t offset) const;

        [[nodiscard]]
        std::size_t size() const noexcept;
    };
}// namespace mjolnir

template<>
struct std::hash<mjolnir::Line> {
    std::size_t operator()(mjolnir::Line const &line) const noexcept;
};

template<>
struct std::hash<mjolnir::internal::SpannedLine> {
    std::size_t operator()(mjolnir::internal::SpannedLine const &spanned_line
    ) const noexcept;
};

#endif//MJOLNIR_SOURCE_H
