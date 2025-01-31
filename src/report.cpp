#include <algorithm>         // for __max_element_fn, max_element
#include <cassert>           // for assert
#include <cstddef>           // for size_t
#include <format>            // for format
#include <iterator>          // for next
#include <mjolnir/report.hpp>// for Report, BasicReportKind, CustomReportKind
#include <optional>          // for optional
#include <set>               // for set, _Rb_tree_const_iterator, operator==
#include <sstream>           // for basic_ostream, char_traits, operator<<
#include <stdexcept>         // for logic_error, out_of_range
#include <string>            // for operator<<, allocator, string, to_string
#include <string_view>       // for operator<<, string_view
#include <utility>           // for move, as_const
#include <variant>           // for get, holds_alternative
#include <vector>            // for vector

#include "mjolnir/color.hpp" // for Color, gray, light_blue, light_cyan
#include "mjolnir/draw.hpp"  // for Characters
#include "mjolnir/source.hpp"// for SpannedLine, Line, Label, Source, Labe...
#include "mjolnir/span.hpp"  // for ColoredSpan, Span

namespace mjolnir {
    namespace report_kind {
        Color to_color(ReportKind const &kind) {
            if (std::holds_alternative<BasicReportKind>(kind)) {
                switch (std::get<BasicReportKind>(kind)) {
                    case BasicReportKind::Error:
                        return colors::light_red;
                    case BasicReportKind::Warning:
                        return colors::light_yellow;
                    case BasicReportKind::Advice:
                        return colors::light_cyan;
                    case BasicReportKind::Continuation:
                        return colors::red;// not that it's used, but whatever
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
                    case BasicReportKind::Continuation:
                        return "Continuation";// not that it's used, but whatever
                }
                throw std::logic_error{"unreachable"};
            }

            return std::get<CustomReportKind>(kind).name;
        }
    }// namespace report_kind

    std::set<Line> Report::get_lines() const {
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

        return lines;
    }

    class ReportPrinter final {
        static constexpr auto line_number_padding_before{1};
        static constexpr auto line_number_padding_after{1};
        static constexpr auto padding_after_vert_bar{1};
        static constexpr auto padding_past_max{2};

        std::ostream  *os_;
        Report const  *report_;
        std::set<Line> lines_{report_->get_lines()};
        std::size_t    max_line_nr_len_{[this] {
            auto const max_line = std::ranges::max_element(
                    std::as_const(lines_),
                    [](Line const &lhs, Line const &rhs) { return lhs < rhs; }
            );
            assert(max_line != lines_.cend());// should not be possible

            return std::to_string(max_line->line_number_).size();
        }()};
        std::string    line_number_space_{[this] {
            return std::string(
                    line_number_padding_before + max_line_nr_len_ +
                            line_number_padding_after,
                    ' '
            );
        }()};
        std::string    padding_after_vert_bar_str_{
                std::string(padding_after_vert_bar, ' ')
        };

        [[nodiscard]]
        Characters const &get_characters() const noexcept {
            return report_->config_.characters;
        }

        [[nodiscard]]
        std::set<internal::SpannedLine> get_spanned_lines() const {
            std::set<internal::SpannedLine> spanned_lines{};

            auto const get_spanned_line{[&](Label const &label,
                                            Line const  &line) {
                auto const span{label.get_span()};
                auto [it,
                      _]{spanned_lines.emplace(internal::SpannedLine{line, {}})
                };

                auto extracted{spanned_lines.extract(it)};

                auto &spans{extracted.value().spans_};

                internal::ColoredSpan label_span{
                        line.get_subspan(span), &label
                };
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

                for (auto it{spans_copy.cbegin()}; it != spans_copy.cend();
                     ++it) {
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

        void print_line_start(std::size_t line_nr) const {
            auto const &characters{get_characters()};

            *os_ << std::string(line_number_padding_before, ' ')
                 << std::format("{: >{}}", line_nr, max_line_nr_len_)
                 << std::string(line_number_padding_after, ' ')
                 << characters.vertical_bar_ << padding_after_vert_bar_str_;
        }

        void print_non_code_line_start() const {
            auto const &characters{get_characters()};

            *os_ << line_number_space_
                 << colors::gray.fg(characters.vertical_interruption_)
                 << padding_after_vert_bar_str_;
        }

        void print_line_segment(
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

        void print_highlight(internal::ColoredSpan const &colored_span) const {
            auto const &characters{get_characters()};
            auto const &[span, label_ptr]{colored_span};
            auto const highlight_size{colored_span.center_offset()};
            auto const highlight_left{[&] {
                std::string highlight{};
                highlight.reserve(
                        characters.highlight_.size() * highlight_size
                );

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
                    *os_, highlight_left + characters.highlight_center_ +
                                  highlight_right
            );
        }

        void
        print_highlight_lines(internal::SpannedLine const &spanned_line) const {
            auto const &characters{get_characters()};
            auto const &[line, colored_spans]{spanned_line};

            int line_pos{0};
            for (auto span_it{colored_spans.cbegin()};
                 span_it != colored_spans.cend(); ++span_it) {
                auto const &[span, label_ptr]{*span_it};

                if (!span_it->is_single_line_highlightable(*report_->source_)) {
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
                         current_offset <
                         max_span_end + span_it->center_offset() -
                                 span_it->span_.size() % 2 + padding_past_max;
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
                    if (!rest_it->is_single_line_highlightable(*report_->source_
                        )) {
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

        void print_highlights(internal::SpannedLine const &spanned_line) const {
            auto const &[line, colored_spans]{spanned_line};

            if (!spanned_line.has_highlightable_span())
                return;

            print_non_code_line_start();

            std::size_t highlight_start{0};
            for (auto const &colored_span : colored_spans) {
                auto const &[span, label_ptr]{colored_span};

                if (!colored_span.is_single_line_highlightable(*report_->source_
                    )) {
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

        void print_line(internal::SpannedLine const &spanned_line) const {
            for (auto const &[line, colored_spans]{spanned_line};
                 auto const &colored_span : colored_spans) {
                print_line_segment(line, colored_span);
            }

            end_line();
        }

        void end_line() const {
            *os_ << '\n';
        }

    public:
        ReportPrinter(std::ostream &os, Report const &report)
            : os_{&os}
            , report_{&report} {
        }

        void print_header() const {
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

            auto const line{report_->source_->get_line_info(report_->start_pos_)
            };
            auto const line_nr{line->line_number_};
            auto const col{line->get_column(report_->start_pos_)};

            *os_ << line_number_space_ << characters.line_top_left_
                 << characters.horizontal_bar_ << characters.box_left_
                 << report_->source_->get_name() << ':' << line_nr << ':' << col
                 << characters.box_right_;
            end_line();
        }

        void print_footer() const {
            auto const &characters{get_characters()};

            for (int i{0}; i < line_number_padding_before + max_line_nr_len_ +
                                       line_number_padding_after;
                 ++i) {
                *os_ << characters.horizontal_bar_;
            }

            *os_ << characters.line_bottom_right_;
            end_line();
        }

        void print_empty_line() const {
            auto const &characters{get_characters()};

            *os_ << line_number_space_ << characters.vertical_bar_ << '\n';
        }

        void print_lines() const {
            auto const spanned_lines{get_spanned_lines()};

            for (auto const &spanned_line : spanned_lines) {
                print_line_start(spanned_line.line_.line_number_);
                print_line(spanned_line);
                print_highlights(spanned_line);
            }
        }

        void print_help() const {
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
    };

    Report::Report(ReportKind kind, Source const &source, std::size_t start_pos)
        : kind_{std::move(kind)}
        , start_pos_{start_pos}
        , source_{&source} {
        if (start_pos >= source.size())
            throw std::out_of_range{"start_pos is out of range"};
    }

    Report &Report::with_code(std::string code) {
        code_ = std::move(code);
        return *this;
    }

    Report &Report::with_message(std::string message) {
        message_ = std::move(message);
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
        label.get_span().verify_validity(*source_);

        labels_.emplace_back(std::move(label));
        return *this;
    }

    Report &Report::with_config(ReportConfig const &config) {
        config_ = config;
        return *this;
    }

    void Report::print(std::ostream &os) const {
        auto const characters{config_.characters};

        ReportPrinter const printer{os, *this};
        printer.print_header();
        printer.print_empty_line();
        printer.print_lines();
        printer.print_help();
        printer.print_footer();
    }
}// namespace mjolnir
