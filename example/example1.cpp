#include <fstream>
#include <mjolnir/report.h>

int main() {
    std::string buffer{"int value = 4 << 1337.f;\n"};

    mjolnir::Source source{"test.c", std::move(buffer)};
    mjolnir::Report report{
            mjolnir::BasicReportKind::Error,
            source,
            "Shift operation on non-integral type(s)",
            {2, 13}
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
