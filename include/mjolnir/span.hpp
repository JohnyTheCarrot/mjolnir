#ifndef MJOLNIR_SPAN_H
#define MJOLNIR_SPAN_H

#include <cstdint>

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
        Span operator+(Span const &other) const;

        Span &operator+=(Span const &other);

        void expand(std::size_t by = 1);

        void set_end(std::size_t end);

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

#endif//MJOLNIR_SPAN_H
