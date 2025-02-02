#include <algorithm>         // for max
#include <cmath>             // for ceil
#include <cstddef>           // for size_t
#include <mjolnir/source.hpp>// for Source, Line, Label, LabelDisplay
#include <mjolnir/span.hpp>  // for Span, ColoredSpan
#include <optional>          // for optional
#include <stdexcept>         // for out_of_range

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

    Span Span::operator+(Span const &other) const {
        return Span{std::min(start_, other.start_), std::max(end_, other.end_)};
    }

    Span &Span::operator+=(Span const &other) {
        *this = *this + other;
        return *this;
    }

    void Span::expand(std::size_t by) {
        end_ += by;
    }

    void Span::set_end(std::size_t end) {
        if (end < start_) {
            throw std::invalid_argument{"End cannot be less than start"};
        }

        end_ = end;
    }

    bool Span::is_multiline(Source const &source) const noexcept {
        auto const start_line{source.get_line_info(start_)};
        auto const end_line{source.get_line_info(end_)};

        if (!start_line.has_value() || !end_line.has_value())
            return false;

        return start_line->line_number_ != end_line->line_number_;
    }

    void Span::verify_validity(Source const &source) const {
        if (start_ == end_) {
            throw std::invalid_argument{"Span cannot be empty"};
        }

        if (start_ >= source.size() || end_ > source.size()) {
            throw std::out_of_range{"Span is out of range of the source buffer"
            };
        }
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
