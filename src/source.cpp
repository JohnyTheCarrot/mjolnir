#include <mjolnir/source.hpp>
#include <sstream>

namespace mjolnir {
    Label::Label(Span const &span)
        : span_{span} {
    }

    Span const &Label::get_span() const noexcept {
        return span_;
    }

    LabelDisplay const &Label::get_display() const noexcept {
        return display_;
    }

    Label &Label::with_message(std::string message) {
        display_.message_ = std::move(message);
        return *this;
    }

    Label &Label::with_color(rang::fg color) {
        display_.color_ = color;
        return *this;
    }

    Span Label::get_subspan(Span const &span) const noexcept {
        auto const start{std::max(span.start(), span_.start())};
        auto const end{std::min(span.end(), span_.end())};

        return Span{start, end};
    }

    bool Line::operator==(Line const &other) const {
        return byte_offset_ == other.byte_offset_ &&
               byte_length_ == other.byte_length_;
    }

    bool Line::operator<(Line const &other) const {
        return byte_offset_ < other.byte_offset_;
    }

    std::size_t Line::end() const noexcept {
        return byte_offset_ + byte_length_;
    }

    Span Line::get_subspan(Span const &span) const noexcept {
        auto const start{std::max(span.start(), byte_offset_)};
        auto const end{std::min(span.end(), this->end())};

        return Span{start, end};
    }

    std::size_t Line::get_column(std::size_t offset) const noexcept {
        return offset - byte_offset_;
    }

    bool internal::SpannedLine::operator==(SpannedLine const &other) const {
        return line_ == other.line_ && spans_ == other.spans_;
    }

    bool internal::SpannedLine::operator<(SpannedLine const &other) const {
        return line_ < other.line_;
    }

    bool internal::SpannedLine::has_highlightable_span() const noexcept {
        return std::ranges::any_of(spans_, [](ColoredSpan const &coloredSpan) {
            return coloredSpan.is_highlight();
        });
    }

    std::size_t internal::SpannedLine::max_span_end() const {
        auto const max_span_end{[this] {
            std::size_t max{0};

            for (auto const &span : spans_) {
                if (!span.is_highlight())
                    continue;

                max = std::max(max, span.span_.end());
            }

            return max;
        }()};

        return max_span_end - line_.byte_offset_;
    }

    Source::Source(std::string name, std::string buffer)
        : name_{std::move(name)}
        , buffer_{std::move(buffer)}
        , lines_{[this] {
            std::vector<Line> lines;

            auto last{buffer_.cbegin()};

            for (auto it{buffer_.cbegin()}; it != buffer_.cend(); ++it) {
                std::string::const_iterator line_it;

                if (*it == '\n') {
                    line_it = it;
                } else if (std::next(it) == buffer_.cend()) {
                    line_it = buffer_.cend();
                } else {
                    continue;
                }

                auto const old_offset{std::distance(buffer_.cbegin(), last)};
                auto const length{std::distance(last, line_it)};
                lines.emplace_back(
                        Line{.byte_offset_ =
                                     static_cast<std::size_t>(old_offset),
                             .byte_length_ = static_cast<std::size_t>(length),
                             .line_number_ = lines.size() + 1}
                );

                last = line_it + 1;
            }

            return lines;
        }()} {
    }

    std::string_view Source::get_name() const noexcept {
        return name_;
    }

    std::optional<std::string_view> Source::get_line(std::size_t offset) const {
        auto const lineInfo{get_line_info(offset)};

        if (!lineInfo.has_value())
            return std::nullopt;

        return get_line(lineInfo.value());
    }

    std::string_view Source::get_line(Line const &line) const {
        return std::string_view{
                buffer_.data() + line.byte_offset_, line.byte_length_
        };
    }

    std::string_view
    Source::get_line(Line const &line, Span const &span) const {
        auto const subspan{line.get_subspan(span)};
        return std::string_view{
                buffer_.data() + subspan.start(), subspan.size()
        };
    }

    std::optional<Line> Source::get_line_info(std::size_t offset) const {
        if (offset >= buffer_.size())
            return std::nullopt;

        // TODO: this here below isn't doing what I want it to do
        auto it{std::upper_bound(
                lines_.cbegin(), lines_.cend(), offset,
                [](std::size_t lhs, Line const &rhs) {
                    return lhs < rhs.byte_offset_;
                }
        )};

        if (it == lines_.cbegin())
            return std::nullopt;

        --it;

        auto const last{lines_.back()};
        if (it == lines_.cend() &&
            offset <= last.byte_offset_ + last.byte_length_)
            return last;

        return *it;
    }
}// namespace mjolnir

std::size_t std::hash<mjolnir::Line>::operator()(mjolnir::Line const &line
) const noexcept {
    return std::hash<std::size_t>{}(line.byte_offset_) ^
           std::hash<std::size_t>{}(line.byte_length_);
}

std::size_t std::hash<mjolnir::internal::SpannedLine>::operator()(
        mjolnir::internal::SpannedLine const &spannedLine
) const noexcept {
    // Purposefully not hashing the spans, so that we can do lookups based on the line
    return std::hash<mjolnir::Line>{}(spannedLine.line_);
}
