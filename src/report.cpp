#include <cassert>
#include <mjolnir/report.h>
#include <sstream>
#include <utility>

namespace mjolnir {
    namespace report_kind {
        rang::fg to_color(ReportKind const &kind) {
            if (std::holds_alternative<BasicReportKind>(kind)) {
                switch (std::get<BasicReportKind>(kind)) {
                    case BasicReportKind::Error:
                        return rang::fg::red;
                    case BasicReportKind::Warning:
                        return rang::fg::yellow;
                    case BasicReportKind::Advice:
                        return rang::fg::cyan;
                }
                throw std::logic_error{"unreachable"};
            }

            auto const &[_, color]{std::get<CustomReportKind>(kind)};
            return color;
        }

        std::string_view to_string(ReportKind const &kind) {
            if (std::holds_alternative<BasicReportKind>(kind)) {
                switch (std::get<BasicReportKind>(kind)) {
                    case BasicReportKind::Error:
                        return "Error";
                    case BasicReportKind::Warning:
                        return "Warning";
                    case BasicReportKind::Advice:
                        return "Advice";
                }
                throw std::logic_error{"unreachable"};
            }

            return std::get<CustomReportKind>(kind).name;
        }
    }// namespace report_kind

    std::set<internal::SpannedLine> Report::get_spanned_lines() const {
        std::set<internal::SpannedLine> spanned_lines{};

        auto const get_spanned_line{[&](Label const &label, Line const &line) {
            auto const span{label.get_span()};
            auto [it, _]{spanned_lines.emplace(internal::SpannedLine{line, {}})
            };

            auto extracted{spanned_lines.extract(it)};

            auto &spans{extracted.value().spans_};

            internal::ColoredSpan label_span{line.get_subspan(span), &label};
            spans.emplace(label_span);

            spanned_lines.insert(std::move(extracted));
        }};
        for (auto const &label : labels_) {
            auto const span{label.get_span()};
            auto const start_line{source_->get_line_info(span.start()).value()};
            auto const end_line{source_->get_line_info(span.end()).value()};
            get_spanned_line(label, start_line);
            get_spanned_line(label, end_line);
        }

        // add the uncolored lines
        for (auto &line : spanned_lines) {
            auto  spans_copy{line.spans_};
            auto  extracted{spanned_lines.extract(line)};
            auto &spans{extracted.value().spans_};
            auto  span_start{line.line_.byte_offset_};
            auto  last_start{span_start};

            for (auto it{spans_copy.cbegin()}; it != spans_copy.cend(); ++it) {
                internal::ColoredSpan const after_span{
                        {last_start, span_start}, nullptr
                };
                if (!after_span.span_.empty())
                    spans.emplace(after_span);

                internal::ColoredSpan const before_span{
                        {span_start, it->span_.start()}, nullptr
                };

                if (!before_span.span_.empty()) {
                    spans.emplace(before_span);
                }

                last_start = span_start;
                span_start = it->span_.end();
            }

            internal::ColoredSpan const after_span{
                    {span_start, line.line_.end()}, nullptr
            };
            if (!after_span.span_.empty())
                spans.emplace(after_span);

            spanned_lines.insert(std::move(extracted));
        }

        return spanned_lines;
    }

    Report::Report(
            ReportKind kind, Source const &source, std::string message,
            Span const &span
    )
        : kind_{std::move(kind)}
        , message_{std::move(message)}
        , span_{span}
        , source_{&source} {
    }

    Report &Report::with_code(std::string code) {
        code_ = std::move(code);
        return *this;
    }

    Report &Report::with_note(std::string note) {
        notes_.emplace_back(std::move(note));
        return *this;
    }

    Report &Report::with_help(std::string help) {
        help_.emplace_back(std::move(help));
        return *this;
    }

    Report &Report::with_label(Label label) {
        labels_.emplace_back(std::move(label));
        return *this;
    }

    Report &Report::with_config(ReportConfig const &config) {
        config_ = config;
        return *this;
    }

    void Report::print(std::ostream &os) const {
        auto const characters{config_.characters};
        auto const color{report_kind::to_color(kind_)};
        auto const kind{report_kind::to_string(kind_)};

        os << color;
        if (code_.has_value()) {
            os << '[' << code_.value() << "] ";
        }
        os << kind << rang::fg::reset << ": " << message_ << '\n';

        std::set<Line> lines{};
        for (auto const &label : labels_) {
            if (auto const startLine{
                        source_->get_line_info(label.get_span().start())
                };
                startLine.has_value())
                lines.emplace(startLine.value());

            if (auto const endLine{source_->get_line_info(label.get_span().end()
                )};
                endLine.has_value())
                lines.emplace(endLine.value());
        }

        auto const max_line = std::ranges::max_element(
                std::as_const(lines),
                [](Line const &lhs, Line const &rhs) { return lhs < rhs; }
        );
        assert(max_line != lines.cend());// should not be possible
        constexpr auto padding_after_line_number{1};
        constexpr auto padding_after_vert_bar{1};
        auto const     padding{std::to_string(max_line->byte_offset_).size()};

        std::string const line_number_padding(
                padding + padding_after_line_number, ' '
        );

        os << line_number_padding << characters.line_top_left_
           << characters.horizontal_bar_ << characters.box_left_
           << span_.to_string(source_->get_name()) << characters.box_right_
           << '\n';

        os << line_number_padding << characters.vertical_bar_ << '\n';

        auto const        spanned_lines{get_spanned_lines()};
        std::string const line_start{
                line_number_padding + characters.vertical_bar_ +
                std::string(padding_after_vert_bar, ' ')
        };

        for (auto it{spanned_lines.cbegin()}; it != spanned_lines.cend();
             ++it) {
            auto const &[line, spans]{*it};

            os << std::format("{: >{}}", line.line_number_, padding)
               << std::string(padding_after_line_number, ' ')
               << characters.vertical_bar_
               << std::string(padding_after_vert_bar, ' ');
            for (auto const &[span, label_ptr] : spans) {
                auto const content{source_->get_line(line, span)};
                if (label_ptr == nullptr) {
                    os << content;
                    continue;
                }

                os << label_ptr->get_display().color_ << content
                   << rang::fg::reset;
            }
            os << '\n';

            std::vector<std::string> lines_and_messages{};

            std::string highlight_padding{
                    line_number_padding + characters.vertical_bar_ +
                    std::string(padding_after_vert_bar, ' ')
            };
            highlight_padding.reserve(line.byte_length_);
            bool        has_written_highlight{false};
            std::size_t max_span_end{0};

            for (auto const &[span, label_ptr] : spans) {
                if (label_ptr == nullptr ||
                    label_ptr->get_span().is_multiline(*source_)) {
                    highlight_padding += std::string(span.size(), ' ');
                    continue;
                }

                max_span_end = std::max(
                        max_span_end,
                        line.byte_length_ - (line.end() - span.end())
                );
                has_written_highlight = true;
                os << rang::fg::gray << highlight_padding << rang::fg::reset;
                highlight_padding.clear();
                auto const highlight_size{
                        std::max(static_cast<int>(span.size()) / 2 - 1, 0)
                };
                std::string highlight;
                highlight.reserve(
                        characters.highlight_.size() * highlight_size
                );
                for (std::size_t i{0}; i < highlight_size; ++i) {
                    highlight += characters.highlight_;
                }

                os << label_ptr->get_display().color_ << highlight
                   << characters.highlight_center_ << highlight;

                if (span.size() % 2 == 0) {
                    os << characters.highlight_;
                }
                os << rang::fg::reset;
            }
            if (has_written_highlight) {
                os << '\n' << rang::fg::gray << line_start << rang::fg::reset;

                int         index{0};
                std::string line_padding{};
                for (auto span_it{spans.cbegin()}; span_it != spans.cend();
                     ++span_it) {
                    auto const &[span, label_ptr]{*span_it};

                    if (label_ptr == nullptr ||
                        label_ptr->get_span().is_multiline(*source_)) {
                        std::string current_line_padding(span.size(), ' ');
                        line_padding += current_line_padding;
                        os << current_line_padding;
                        continue;
                    }

                    auto const highlight_center_offset{
                            std::max(static_cast<int>(span.size()) / 2, 0)
                    };
                    std::string current_highlight_offset_str(
                            highlight_center_offset, ' '
                    );
                    os << current_highlight_offset_str
                       << label_ptr->get_display().color_
                       << characters.line_bottom_left_;
                    if (span_it == spans.cend()) {
                        break;
                    }
                    for (auto current_offset{
                                 line_padding.size() + highlight_center_offset
                         };
                         current_offset < max_span_end; ++current_offset) {
                        os << characters.horizontal_bar_;
                    }

                    if (label_ptr->get_display().message_.has_value()) {
                        os << rang::fg::reset << ' '
                           << label_ptr->get_display().message_.value();
                    }

                    os << '\n'
                       << rang::fg::gray << line_start << rang::fg::reset
                       << line_padding << current_highlight_offset_str;

                    bool did_draw_lines{false};
                    for (auto rest_it{std::next(span_it)};
                         rest_it != spans.cend(); ++rest_it) {
                        auto const &[rest_span, rest_label_ptr]{*rest_it};
                        if (rest_label_ptr == nullptr ||
                            rest_label_ptr->get_span().is_multiline(*source_)) {
                            os << std::string(rest_span.size(), ' ');
                            continue;
                        }
                        did_draw_lines = true;

                        auto const highlight_center_offset{std::max(
                                static_cast<int>(rest_span.size()) / 2, 0
                        )};
                        os << std::string(highlight_center_offset, ' ');
                        os << rest_label_ptr->get_display().color_
                           << characters.vertical_bar_;
                    }
                    if (did_draw_lines) {
                        os << '\n'
                           << rang::fg::gray << line_start << rang::fg::reset
                           << line_padding << current_highlight_offset_str;
                    }

                    ++index;
                }
                os << '\n';
            }
        }
        os << line_number_padding << characters.vertical_bar_ << '\n';

        // TODO: help, etc
        for (auto const &help : help_) {
            os << rang::fg::gray << line_start << rang::fg::cyan
               << "Help: " << rang::fg::reset << help << '\n';
        }

        for (int i{0}; i < padding + 1; ++i) {
            os << characters.horizontal_bar_;
        }

        os << characters.line_bottom_right_ << '\n';
    }
}// namespace mjolnir
