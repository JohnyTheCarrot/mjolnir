#include <fstream>
#include <mjolnir/report.h>

void print_warning(mjolnir::Source const &source) {
    mjolnir::Report report{
            mjolnir::BasicReportKind::Warning, source,
            "Dereference on a null pointer", 75
    };
    report.with_code("W16")
            // a label without a message will just include the line
            .with_label(mjolnir::Label{{57, 70}})
            .with_label(mjolnir::Label{{94, 95}})
            .with_label(
                    mjolnir::Label{{75, 79}}
                            .with_message("Dereference occurs here")
                            .with_color(rang::fg::cyan)
            )
            .with_label(
                    mjolnir::Label{{47, 54}}
                            .with_message("Initialized as nullptr here")
                            .with_color(rang::fg::magenta)
            )
            .with_config(mjolnir::ReportConfig{mjolnir::characters::ascii});

    report.print(std::cout);
}

void print_error(mjolnir::Source const &source) {
    mjolnir::Report report{
            mjolnir::BasicReportKind::Error, source,
            "Shift operation on non-integral type(s)", 12
    };
    report.with_label(
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
        
float *ptr = nullptr;

void main() {
    *ptr = 40.f + 2.f;
}
)"
    };

    mjolnir::Source const source{"test.c", std::move(buffer)};

    print_error(source);
    print_warning(source);
}
