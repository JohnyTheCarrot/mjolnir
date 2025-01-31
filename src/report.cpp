#include "mjolnir/report.hpp"// for Report, BasicReportKind, CustomReportKind

#include <cstddef>    // for size_t
#include <iosfwd>     // for ostream
#include <optional>   // for optional
#include <set>        // for set
#include <stdexcept>  // for logic_error, out_of_range
#include <string>     // for string, char_traits
#include <string_view>// for string_view
#include <utility>    // for move
#include <variant>    // for get, holds_alternative
#include <vector>     // for vector

#include "mjolnir/color.hpp" // for Color, light_cyan, light_red, light_ye...
#include "mjolnir/draw.hpp"  // for Characters
#include "mjolnir/source.hpp"// for Label, Line, Source
#include "mjolnir/span.hpp"  // for Span
#include "report_printer.h"  // for ReportPrinter

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
