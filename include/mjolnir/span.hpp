#ifndef SPAN_H
#define SPAN_H

#include <cstddef>
#include <string>

#include "rang.hpp"

namespace mjolnir {
    class Label;
    class Source;

    class Span final {
        std::size_t start_;
        std::size_t end_;

    public:
        Span(std::size_t start, std::size_t end);

        [[nodiscard]]
        std::size_t size() const noexcept;

        [[nodiscard]]
        bool empty() const noexcept;

        [[nodiscard]]
        std::size_t start() const noexcept;

        [[nodiscard]]
        std::size_t end() const noexcept;

        [[nodiscard]]
        bool operator==(Span const &other) const noexcept;

        [[nodiscard]]
        bool operator<(Span const &other) const noexcept;

        [[nodiscard]]
        bool is_multiline(Source const &source) const noexcept;

        void verify_validity(Source const &source) const;
    };

    namespace internal {
        struct ColoredSpan final {
            Span         span_;
            Label const *label_ptr_;

            [[nodiscard]]
            bool operator==(ColoredSpan const &other) const;

            [[nodiscard]]
            bool operator<(ColoredSpan const &other) const;

            [[nodiscard]]
            std::size_t center_offset() const noexcept;

            [[nodiscard]]
            bool is_highlight() const;

            [[nodiscard]]
            bool
            is_single_line_highlightable(Source const &source) const noexcept;
        };
    }// namespace internal
}// namespace mjolnir

#endif//SPAN_H
