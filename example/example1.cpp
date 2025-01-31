#include <fstream>
#include <mjolnir/report.h>

void print_warning(mjolnir::Source const &source) {
    mjolnir::Report report{mjolnir::BasicReportKind::Warning, source, 44};
    report.with_code("W16")
            // a label without a message will just include the line
            .with_message("Dereference on a null pointer")
            .with_label(mjolnir::Label{{26, 39}})
            .with_label(mjolnir::Label{{63, 64}})
            .with_label(
                    mjolnir::Label{{44, 48}}
                            .with_message("Dereference occurs here")
                            .with_color(rang::fg::cyan)
            );

    report.print(std::cout);


    std::string buffer{"float *ptr = nullptr;"};

    mjolnir::Source const other_source{"incl.h", std::move(buffer)};
    mjolnir::Report       other_file_report{
            mjolnir::BasicReportKind::Continuation, other_source, 13
    };

    other_file_report.with_label(
            mjolnir::Label{{13, 20}}
                    .with_message("Initialized as nullptr here")
                    .with_color(rang::fg::cyan)
    );
    other_file_report.print(std::cout);
}

void print_error(mjolnir::Source const &source) {
    mjolnir::Report report{mjolnir::BasicReportKind::Error, source, 12};
    report.with_message("Shift operation on non-integral type(s)")
            .with_label(
                    mjolnir::Label{{12, 13}}
                            .with_color(rang::fg::green)
                            .with_message("This is of type int")
            )
            .with_label(
                    mjolnir::Label{{17, 23}}
                            .with_color(rang::fg::magenta)
                            .with_message("This is of type float")
            )
            .with_code("E03")
            .with_help(
                    "Only integral types can be used as operands of a shift "
                    "operation, consider casting the operands to an integral "
                    "type."
            );

    report.print(std::cout);
}

int main() {
    std::string buffer{
            R"(int value = 4 << 1337.f;

void main() {
    *ptr = 40.f + 2.f;
}
)"
    };

    mjolnir::Source const source{"test.c", std::move(buffer)};

    print_error(source);
    print_warning(source);
}
