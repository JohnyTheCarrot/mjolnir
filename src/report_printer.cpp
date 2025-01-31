#include "report_printer.h"

#include <format>     // for format
#include <iterator>   // for next
#include <optional>   // for optional
#include <string_view>// for operator<<
#include <variant>    // for get, holds_alternative
#include <vector>     // for vector

#include "mjolnir/color.hpp" // for Color, gray, light_blue, light_cyan
#include "mjolnir/draw.hpp"  // for Characters
#include "mjolnir/report.hpp"// for Report, to_color, to_string, BasicRepo...
#include "mjolnir/span.hpp"  // for ColoredSpan, Span

namespace mjolnir {
    Characters const &ReportPrinter::get_characters() const noexcept {
        return report_->config_.characters;
    }

    std::set<internal::SpannedLine> ReportPrinter::get_spanned_lines() const {
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
        for (auto const &label : report_->labels_) {
            auto const span{label.get_span()};
            auto const start_line{
                    report_->source_->get_line_info(span.start()).value()
            };
            auto const end_line{
                    report_->source_->get_line_info(span.end()).value()
            };
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

    void ReportPrinter::print_line_start(std::size_t line_nr) const {
        auto const &characters{get_characters()};

        *os_ << std::string(line_number_padding_before, ' ')
             << std::format("{: >{}}", line_nr, max_line_nr_len_)
             << std::string(line_number_padding_after, ' ')
             << characters.vertical_bar_ << padding_after_vert_bar_str_;
    }

    void ReportPrinter::print_non_code_line_start() const {
        auto const &characters{get_characters()};

        *os_ << line_number_space_
             << colors::gray.fg(characters.vertical_interruption_)
             << padding_after_vert_bar_str_;
    }

    void ReportPrinter::print_line_segment(
            Line const &line, internal::ColoredSpan const &colored_span
    ) const {
        auto const &[span, label_ptr]{colored_span};

        auto const content{report_->source_->get_line(line, span)};
        if (label_ptr == nullptr) {
            *os_ << content;
            return;
        }

        label_ptr->get_display().print(*os_, content);
    }

    void ReportPrinter::print_highlight(
            internal::ColoredSpan const &colored_span
    ) const {
        auto const &characters{get_characters()};
        auto const &[span, label_ptr]{colored_span};
        auto const highlight_size{colored_span.center_offset()};
        auto const highlight_left{[&] {
            std::string highlight{};
            highlight.reserve(characters.highlight_.size() * highlight_size);

            for (std::size_t i{0}; i < highlight_size; ++i) {
                highlight += characters.highlight_;
            }

            return highlight;
        }()};
        auto const highlight_right{[&] {
            std::string highlight{highlight_left};
            if (span.size() % 2 == 0) {
                highlight += characters.highlight_;
            }

            return highlight;
        }()};

        label_ptr->get_display().print(
                *os_,
                highlight_left + characters.highlight_center_ + highlight_right
        );
    }

    void ReportPrinter::print_highlight_lines(
            internal::SpannedLine const &spanned_line
    ) const {
        auto const &characters{get_characters()};
        auto const &[line, colored_spans]{spanned_line};

        int line_pos{0};
        for (auto span_it{colored_spans.cbegin()};
             span_it != colored_spans.cend(); ++span_it) {
            auto const &[span, label_ptr]{*span_it};

            if (!span_it->is_highlight() ||
                !span_it->is_single_line_highlightable(*report_->source_)) {
                line_pos += span.size();
                continue;
            }

            print_non_code_line_start();

            {
                auto const center_offset{span_it->center_offset()};
                *os_ << std::string(line_pos + center_offset, ' ');
                line_pos += span_it->span_.size();

                auto const &display{label_ptr->get_display()};
                *os_ << display.color_->fg_start()
                     << characters.line_bottom_left_;

                auto const max_span_end{spanned_line.max_span_end()};
                for (auto current_offset{line_pos};
                     current_offset < max_span_end + span_it->center_offset() -
                                              span_it->span_.size() % 2 +
                                              padding_past_max;
                     ++current_offset) {
                    *os_ << characters.horizontal_bar_;
                }

                *os_ << Color::end << ' ' << display.message_.value();
            }

            end_line();

            print_non_code_line_start();

            int rest_line_padding{line_pos};
            for (auto rest_it{std::next(span_it)};
                 rest_it != colored_spans.cend(); ++rest_it) {
                auto const &[rest_span, rest_label_ptr]{*rest_it};
                if (!rest_it->is_single_line_highlightable(*report_->source_)) {
                    rest_line_padding += rest_span.size();
                    continue;
                }

                auto const center_offset{rest_it->center_offset()};
                *os_ << std::string(rest_line_padding + center_offset, ' ');
                rest_line_padding = rest_span.size() - 1;

                *os_ << rest_label_ptr->get_display().color_->fg(
                        characters.vertical_bar_
                );
            }
            end_line();
        }
    }

    void ReportPrinter::print_highlights(
            internal::SpannedLine const &spanned_line
    ) const {
        auto const &[line, colored_spans]{spanned_line};

        if (!spanned_line.has_highlightable_span())
            return;

        print_non_code_line_start();

        std::size_t highlight_start{0};
        for (auto const &colored_span : colored_spans) {
            auto const &[span, label_ptr]{colored_span};

            if (!colored_span.is_single_line_highlightable(*report_->source_)) {
                highlight_start += span.size();
                continue;
            }

            *os_ << std::string(highlight_start, ' ');
            highlight_start = 0;
            print_highlight(colored_span);
        }
        end_line();
        print_highlight_lines(spanned_line);
    }

    void
    ReportPrinter::print_line(internal::SpannedLine const &spanned_line) const {
        for (auto const &[line, colored_spans]{spanned_line};
             auto const &colored_span : colored_spans) {
            print_line_segment(line, colored_span);
        }

        end_line();
    }

    void ReportPrinter::end_line() const {
        *os_ << '\n';
    }

    void ReportPrinter::print_header() const {
        auto const &characters{get_characters()};

        auto const color{report_kind::to_color(report_->kind_)};
        auto const kind{report_kind::to_string(report_->kind_)};

        if (std::holds_alternative<BasicReportKind>(report_->kind_) &&
            std::get<BasicReportKind>(report_->kind_) !=
                    BasicReportKind::Continuation) {
            *os_ << color.fg_start();
            if (report_->code_.has_value()) {
                *os_ << '[' << report_->code_.value() << "] ";
            }
            *os_ << kind << Color::end;
            if (report_->message_.has_value()) {
                *os_ << ": " << report_->message_.value();
            }
            end_line();
        }

        auto const line{report_->source_->get_line_info(report_->start_pos_)};
        auto const line_nr{line->line_number_};
        auto const col{line->get_column(report_->start_pos_)};

        *os_ << line_number_space_ << characters.line_top_left_
             << characters.horizontal_bar_ << characters.box_left_
             << report_->source_->get_name() << ':' << line_nr << ':' << col
             << characters.box_right_;
        end_line();
    }

    void ReportPrinter::print_footer() const {
        auto const &characters{get_characters()};

        for (int i{0}; i < line_number_padding_before + max_line_nr_len_ +
                                   line_number_padding_after;
             ++i) {
            *os_ << characters.horizontal_bar_;
        }

        *os_ << characters.line_bottom_right_;
        end_line();
    }

    void ReportPrinter::print_empty_line() const {
        auto const &characters{get_characters()};

        *os_ << line_number_space_ << characters.vertical_bar_ << '\n';
    }

    void ReportPrinter::print_lines() const {
        auto const spanned_lines{get_spanned_lines()};

        for (auto const &spanned_line : spanned_lines) {
            print_line_start(spanned_line.line_.line_number_);
            print_line(spanned_line);
            print_highlights(spanned_line);
        }
    }

    void ReportPrinter::print_help() const {
        for (auto const &help : report_->help_) {
            print_non_code_line_start();
            *os_ << colors::light_blue.fg_start() << "Help: " << Color::end
                 << help;
            end_line();
        }

        for (auto const &note : report_->notes_) {
            print_non_code_line_start();
            *os_ << colors::light_cyan.fg_start() << "Note: " << Color::end
                 << note;
            end_line();
        }
    }
}// namespace mjolnir
