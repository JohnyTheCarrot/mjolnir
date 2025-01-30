#ifndef REPORT_H
#define REPORT_H

#include <optional>
#include <set>
#include <string>
#include <variant>
#include <vector>

#include "draw.h"
#include "source.hpp"
#include "span.hpp"

namespace mjolnir {
    enum class BasicReportKind { Error, Warning, Advice };

    struct CustomReportKind final {
        std::string name;
        rang::fg    color;
    };

    using ReportKind = std::variant<BasicReportKind, CustomReportKind>;

    namespace report_kind {
        [[nodiscard]]
        rang::fg to_color(ReportKind const &kind);

        [[nodiscard]]
        std::string_view to_string(ReportKind const &kind);
    }// namespace report_kind

    struct ReportConfig final {
        Characters characters{characters::unicode};
    };

    class Report final {
        ReportKind    kind_;
        std::string   message_;
        std::size_t   start_pos_;
        Source const *source_;

        std::optional<std::string> code_;
        std::vector<std::string>   notes_;
        std::vector<std::string>   help_;
        std::vector<Label>         labels_;
        ReportConfig               config_{};

        [[nodiscard]]
        std::set<Line> get_lines() const;

        friend class ReportPrinter;

    public:
        Report(ReportKind kind, Source const &source, std::string message,
               std::size_t start_pos);

        Report &with_code(std::string code);

        Report &with_note(std::string note);

        Report &with_help(std::string help);

        Report &with_label(Label label);

        Report &with_config(ReportConfig const &config);

        void print(std::ostream &os) const;
    };
}// namespace mjolnir

#endif//REPORT_H
