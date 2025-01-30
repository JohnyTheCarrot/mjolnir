#include <cmath>
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

        std::size_t ColoredSpan::center_offset() const noexcept {
            auto const size_float{static_cast<float>(span_.size())};
            auto const ceiled_half_size{
                    static_cast<int>(std::ceil(size_float / 2.f))
            };

            return std::max(ceiled_half_size - 1, 0);
        }

        bool ColoredSpan::is_highlight() const {
            return label_ptr_ != nullptr &&
                   label_ptr_->get_display().message_.has_value();
        }

        bool ColoredSpan::is_single_line_highlightable(
                Source const &source
        ) const noexcept {
            return label_ptr_ != nullptr && !span_.is_multiline(source);
        }
    }// namespace internal
}// namespace mjolnir
