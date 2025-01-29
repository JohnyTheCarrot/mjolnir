#ifndef DRAW_H
#define DRAW_H

#include <string>

#include "rang.hpp"

namespace mjolnir {
    struct Characters final {
        std::string horizontal_bar_;
        std::string vertical_bar_;
        std::string crossing_;
        std::string arrow_up_;
        std::string arrow_right_;
        std::string line_top_left_;
        std::string line_top_right_;
        std::string line_top_middle_;
        std::string line_bottom_left_;
        std::string line_bottom_right_;
        std::string line_bottom_middle_;
        std::string branch_left_;
        std::string branch_right_;
        std::string highlight_center_;
        std::string highlight_;
        std::string box_left_;
        std::string box_right_;
    };

    namespace characters {
        Characters const unicode{
                .horizontal_bar_     = "─",
                .vertical_bar_       = "│",
                .crossing_           = "┼",
                .arrow_up_           = "▲",
                .arrow_right_        = "▶",
                .line_top_left_      = "╭",
                .line_top_right_     = "╮",
                .line_top_middle_    = "┬",
                .line_bottom_left_   = "╰",
                .line_bottom_right_  = "╯",
                .line_bottom_middle_ = "┴",
                .branch_left_        = "├",
                .branch_right_       = "┤",
                .highlight_center_   = "┬",
                .highlight_          = "─",
                .box_left_           = "[",
                .box_right_          = "]",
        };

        Characters const ascii{
                .horizontal_bar_     = "-",
                .vertical_bar_       = "|",
                .crossing_           = "+",
                .arrow_up_           = "^",
                .arrow_right_        = ">",
                .line_top_left_      = ",",
                .line_top_right_     = ".",
                .line_top_middle_    = "v",
                .line_bottom_left_   = "`",
                .line_bottom_right_  = "'",
                .line_bottom_middle_ = "^",
                .branch_left_        = "|",
                .branch_right_       = "|",
                .highlight_center_   = "|",
                .highlight_          = "^",
                .box_left_           = "[",
                .box_right_          = "]",
        };
    }// namespace characters

    std::array constexpr colors{
            rang::fg::red,  rang::fg::green,   rang::fg::yellow,
            rang::fg::blue, rang::fg::magenta, rang::fg::cyan,
    };
}// namespace mjolnir

#endif//DRAW_H
