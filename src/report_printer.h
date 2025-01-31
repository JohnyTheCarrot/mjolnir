#ifndef REPORT_PRINTER_H
#define REPORT_PRINTER_H

#include <algorithm>         // for max_element
#include <cassert>           // for assert
#include <cstddef>           // for size_t
#include <mjolnir/report.hpp>// for Report
#include <set>               // for operator==, set
#include <sstream>           // for ostream
#include <string>            // for string, to_string
#include <utility>           // for as_const

#include "mjolnir/source.hpp"// for Line, SpannedLine

namespace mjolnir {
    namespace internal {
        struct ColoredSpan;
    }// namespace internal
    struct Characters;

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
        Characters const &get_characters() const noexcept;

        [[nodiscard]]
        std::set<internal::SpannedLine> get_spanned_lines() const;

        void print_line_start(std::size_t line_nr) const;

        void print_non_code_line_start() const;

        void print_line_segment(
                Line const &line, internal::ColoredSpan const &colored_span
        ) const;

        void print_highlight(internal::ColoredSpan const &colored_span) const;

        void print_highlight_lines(internal::SpannedLine const &spanned_line
        ) const;

        void print_highlights(internal::SpannedLine const &spanned_line) const;

        void print_line(internal::SpannedLine const &spanned_line) const;

        void end_line() const;

    public:
        ReportPrinter(std::ostream &os, Report const &report)
            : os_{&os}
            , report_{&report} {
        }

        void print_header() const;

        void print_footer() const;

        void print_empty_line() const;

        void print_lines() const;

        void print_help() const;
    };
}// namespace mjolnir

#endif//REPORT_PRINTER_H
