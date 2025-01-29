#include <format>
#include <mjolnir/source.hpp>
#include <mjolnir/span.hpp>

namespace mjolnir {
    Span::Span(std::size_t start, std::size_t end)
        : start_{start}
        , end_{end} {
    }

    std::size_t Span::size() const noexcept {
        return end_ - start_;
    }

    bool Span::empty() const noexcept {
        return size() == 0;
    }

    std::size_t Span::start() const noexcept {
        return start_;
    }

    std::size_t Span::end() const noexcept {
        return end_;
    }

    std::string Span::to_string(std::string_view file_name) const {
        return std::format("{}:{}:{}", file_name, start_, end_);
    }

    bool Span::operator==(Span const &other) const noexcept {
        return start_ == other.start_ && end_ == other.end_;
    }

    bool Span::operator<(Span const &other) const noexcept {
        return start_ < other.start_;
    }

    bool Span::is_multiline(Source const &source) const noexcept {
        auto const start_line{source.get_line_info(start_)};
        auto const end_line{source.get_line_info(end_)};

        if (!start_line.has_value() || !end_line.has_value())
            return false;

        return start_line->line_number_ != end_line->line_number_;
    }

    namespace internal {
        bool ColoredSpan::operator==(ColoredSpan const &other) const {
            return span_ == other.span_ && label_ptr_ == other.label_ptr_;
        }

        bool ColoredSpan::operator<(ColoredSpan const &other) const {
            return span_ < other.span_;
        }
    }// namespace internal
}// namespace mjolnir
