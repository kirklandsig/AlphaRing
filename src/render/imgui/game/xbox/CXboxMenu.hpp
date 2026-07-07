#pragma once

#include <imgui.h>
#include "./assets/colors.h"
#include <string>
#include <vector>
#include <optional>
#include <iostream>

// option types:
// - 0 = boolean
// - 1 = subpages
// - 2 = positive increment
// - 3 = negative increment
// - 4 = pointer display

enum class OptionType {
    Boolean,
    TeamToggle,
    Subpage,
    SubpageOption,
    Increment,
    Decrement,
    PointerDisplay
};

class Option {
public:
    std::string label;
    OptionType type;
    std::vector<Option> subOptions;
    int subOptionType = 0;
    int value = 0;
    std::optional<ImU32> colorValue;

    std::optional<int*> pointer;
    
    Option(std::string lbl, OptionType t)
        : label(std::move(lbl)), type(t) {}
    
    Option(std::string lbl, OptionType t, int s)
    : label(std::move(lbl)), type(t), subOptionType(s) {}

    Option(std::string lbl, OptionType t, int s, ImU32 c)
        : label(std::move(lbl)), type(t), subOptionType(s), colorValue(c) {}

    // Option(std::string lbl, std::vector<std::string> subs)
    //     : label(std::move(lbl)),
    //       type(OptionType::Subpage),
    //       subOptions(std::move(subs)) {}

    // Option(std::string lbl, int* ptr)
    //     : label(std::move(lbl)),
    //       type(OptionType::PointerDisplay),
    //       pointer(ptr) {}

    void addOption(const Option& opt) {
        subOptions.push_back(opt);
    }
};

class Page {
public:
    std::string label;
    std::vector<Option> options;
    bool visible = false;

    Page() = default;

    explicit Page(std::string lbl, bool vis = false)
        : label(std::move(lbl)), visible(vis) {}

    void addOption(const Option& opt) {
        options.push_back(opt);
    }
};

class Menu {
public:
    static constexpr int MAX_PAGES = 5;
    Page pages[MAX_PAGES];

    Menu() {
        init();
    }

private:
    void initPlayer(int index) {
        pages[index].label = "Player " + std::to_string(index);

        pages[index].addOption(
            Option("Controller", OptionType::Subpage));
        pages[index].options[0].addOption(Option("Controller 1", OptionType::SubpageOption, 0));
        pages[index].options[0].addOption(Option("Controller 2", OptionType::SubpageOption, 0));
        pages[index].options[0].addOption(Option("Controller 3", OptionType::SubpageOption, 0));
        pages[index].options[0].addOption(Option("Controller 4", OptionType::SubpageOption, 0));

        pages[index].addOption(Option("Red Team", OptionType::TeamToggle, 0));

        //
        // adding primary colors
        //
        pages[index].addOption(Option("Primary Color", OptionType::Subpage, 1));
        pages[index].options[2].addOption(Option("Steel", OptionType::SubpageOption, 1, defaultColors[0]));
        pages[index].options[2].addOption(Option("Silver", OptionType::SubpageOption, 1, defaultColors[1]));
        pages[index].options[2].addOption(Option("White", OptionType::SubpageOption, 1, defaultColors[2]));
        pages[index].options[2].addOption(Option("Brown", OptionType::SubpageOption, 1, defaultColors[3]));
        pages[index].options[2].addOption(Option("Tan", OptionType::SubpageOption, 1, defaultColors[4]));
        pages[index].options[2].addOption(Option("Khaki", OptionType::SubpageOption, 1, defaultColors[5]));
        pages[index].options[2].addOption(Option("Sage", OptionType::SubpageOption, 1, defaultColors[6]));
        pages[index].options[2].addOption(Option("Olive", OptionType::SubpageOption, 1, defaultColors[7]));
        pages[index].options[2].addOption(Option("Drab", OptionType::SubpageOption, 1, defaultColors[8]));
        pages[index].options[2].addOption(Option("Forest", OptionType::SubpageOption, 1, defaultColors[9]));
        pages[index].options[2].addOption(Option("Green", OptionType::SubpageOption, 1, defaultColors[10]));
        pages[index].options[2].addOption(Option("Sea Foam", OptionType::SubpageOption, 1, defaultColors[11]));
        pages[index].options[2].addOption(Option("Teal", OptionType::SubpageOption, 1, defaultColors[12]));
        pages[index].options[2].addOption(Option("Aqua", OptionType::SubpageOption, 1, defaultColors[13]));
        pages[index].options[2].addOption(Option("Cyan", OptionType::SubpageOption, 1, defaultColors[14]));
        pages[index].options[2].addOption(Option("Blue", OptionType::SubpageOption, 1, defaultColors[15]));
        pages[index].options[2].addOption(Option("Cobalt", OptionType::SubpageOption, 1, defaultColors[16]));
        pages[index].options[2].addOption(Option("Ice", OptionType::SubpageOption, 1, defaultColors[17]));
        pages[index].options[2].addOption(Option("Violet", OptionType::SubpageOption, 1, defaultColors[18]));
        pages[index].options[2].addOption(Option("Orchid", OptionType::SubpageOption, 1, defaultColors[19]));
        pages[index].options[2].addOption(Option("Lavender", OptionType::SubpageOption, 1, defaultColors[20]));
        pages[index].options[2].addOption(Option("Maroon", OptionType::SubpageOption, 1, defaultColors[21]));
        pages[index].options[2].addOption(Option("Brick", OptionType::SubpageOption, 1, defaultColors[22]));
        pages[index].options[2].addOption(Option("Rose", OptionType::SubpageOption, 1, defaultColors[23]));
        pages[index].options[2].addOption(Option("Rust", OptionType::SubpageOption, 1, defaultColors[24]));
        pages[index].options[2].addOption(Option("Coral", OptionType::SubpageOption, 1, defaultColors[25]));
        pages[index].options[2].addOption(Option("Peach", OptionType::SubpageOption, 1, defaultColors[26]));
        pages[index].options[2].addOption(Option("Gold", OptionType::SubpageOption, 1, defaultColors[27]));
        pages[index].options[2].addOption(Option("Yellow", OptionType::SubpageOption, 1, defaultColors[28]));
        pages[index].options[2].addOption(Option("Pale", OptionType::SubpageOption, 1, defaultColors[29]));
        //
        // adding secondary colors
        //
        pages[index].addOption(Option("Secondary Color", OptionType::Subpage, 2));
        pages[index].options[3].addOption(Option("Steel", OptionType::SubpageOption, 2, defaultColors[0]));
        pages[index].options[3].addOption(Option("Silver", OptionType::SubpageOption, 2, defaultColors[1]));
        pages[index].options[3].addOption(Option("White", OptionType::SubpageOption, 2, defaultColors[2]));
        pages[index].options[3].addOption(Option("Brown", OptionType::SubpageOption, 2, defaultColors[3]));
        pages[index].options[3].addOption(Option("Tan", OptionType::SubpageOption, 2, defaultColors[4]));
        pages[index].options[3].addOption(Option("Khaki", OptionType::SubpageOption, 2, defaultColors[5]));
        pages[index].options[3].addOption(Option("Sage", OptionType::SubpageOption, 2, defaultColors[6]));
        pages[index].options[3].addOption(Option("Olive", OptionType::SubpageOption, 2, defaultColors[7]));
        pages[index].options[3].addOption(Option("Drab", OptionType::SubpageOption, 2, defaultColors[8]));
        pages[index].options[3].addOption(Option("Forest", OptionType::SubpageOption, 2, defaultColors[9]));
        pages[index].options[3].addOption(Option("Green", OptionType::SubpageOption, 2, defaultColors[10]));
        pages[index].options[3].addOption(Option("Sea Foam", OptionType::SubpageOption, 2, defaultColors[11]));
        pages[index].options[3].addOption(Option("Teal", OptionType::SubpageOption, 2, defaultColors[12]));
        pages[index].options[3].addOption(Option("Aqua", OptionType::SubpageOption, 2, defaultColors[13]));
        pages[index].options[3].addOption(Option("Cyan", OptionType::SubpageOption, 2, defaultColors[14]));
        pages[index].options[3].addOption(Option("Blue", OptionType::SubpageOption, 2, defaultColors[15]));
        pages[index].options[3].addOption(Option("Cobalt", OptionType::SubpageOption, 2, defaultColors[16]));
        pages[index].options[3].addOption(Option("Ice", OptionType::SubpageOption, 2, defaultColors[17]));
        pages[index].options[3].addOption(Option("Violet", OptionType::SubpageOption, 2, defaultColors[18]));
        pages[index].options[3].addOption(Option("Orchid", OptionType::SubpageOption, 2, defaultColors[19]));
        pages[index].options[3].addOption(Option("Lavender", OptionType::SubpageOption, 2, defaultColors[20]));
        pages[index].options[3].addOption(Option("Maroon", OptionType::SubpageOption, 2, defaultColors[21]));
        pages[index].options[3].addOption(Option("Brick", OptionType::SubpageOption, 2, defaultColors[22]));
        pages[index].options[3].addOption(Option("Rose", OptionType::SubpageOption, 2, defaultColors[23]));
        pages[index].options[3].addOption(Option("Rust", OptionType::SubpageOption, 2, defaultColors[24]));
        pages[index].options[3].addOption(Option("Coral", OptionType::SubpageOption, 2, defaultColors[25]));
        pages[index].options[3].addOption(Option("Peach", OptionType::SubpageOption, 2, defaultColors[26]));
        pages[index].options[3].addOption(Option("Gold", OptionType::SubpageOption, 2, defaultColors[27]));
        pages[index].options[3].addOption(Option("Yellow", OptionType::SubpageOption, 2, defaultColors[28]));
        pages[index].options[3].addOption(Option("Pale", OptionType::SubpageOption, 2, defaultColors[29]));
        //
        // adding tertiary colors
        //
        pages[index].addOption(Option("Tertiary Color", OptionType::Subpage, 3));
        pages[index].options[4].addOption(Option("Steel", OptionType::SubpageOption, 3, defaultColors[0]));
        pages[index].options[4].addOption(Option("Silver", OptionType::SubpageOption, 3, defaultColors[1]));
        pages[index].options[4].addOption(Option("White", OptionType::SubpageOption, 3, defaultColors[2]));
        pages[index].options[4].addOption(Option("Brown", OptionType::SubpageOption, 3, defaultColors[3]));
        pages[index].options[4].addOption(Option("Tan", OptionType::SubpageOption, 3, defaultColors[4]));
        pages[index].options[4].addOption(Option("Khaki", OptionType::SubpageOption, 3, defaultColors[5]));
        pages[index].options[4].addOption(Option("Sage", OptionType::SubpageOption, 3, defaultColors[6]));
        pages[index].options[4].addOption(Option("Olive", OptionType::SubpageOption, 3, defaultColors[7]));
        pages[index].options[4].addOption(Option("Drab", OptionType::SubpageOption, 3, defaultColors[8]));
        pages[index].options[4].addOption(Option("Forest", OptionType::SubpageOption, 3, defaultColors[9]));
        pages[index].options[4].addOption(Option("Green", OptionType::SubpageOption, 3, defaultColors[10]));
        pages[index].options[4].addOption(Option("Sea Foam", OptionType::SubpageOption, 3, defaultColors[11]));
        pages[index].options[4].addOption(Option("Teal", OptionType::SubpageOption, 3, defaultColors[12]));
        pages[index].options[4].addOption(Option("Aqua", OptionType::SubpageOption, 3, defaultColors[13]));
        pages[index].options[4].addOption(Option("Cyan", OptionType::SubpageOption, 3, defaultColors[14]));
        pages[index].options[4].addOption(Option("Blue", OptionType::SubpageOption, 3, defaultColors[15]));
        pages[index].options[4].addOption(Option("Cobalt", OptionType::SubpageOption, 3, defaultColors[16]));
        pages[index].options[4].addOption(Option("Ice", OptionType::SubpageOption, 3, defaultColors[17]));
        pages[index].options[4].addOption(Option("Violet", OptionType::SubpageOption, 3, defaultColors[18]));
        pages[index].options[4].addOption(Option("Orchid", OptionType::SubpageOption, 3, defaultColors[19]));
        pages[index].options[4].addOption(Option("Lavender", OptionType::SubpageOption, 3, defaultColors[20]));
        pages[index].options[4].addOption(Option("Maroon", OptionType::SubpageOption, 3, defaultColors[21]));
        pages[index].options[4].addOption(Option("Brick", OptionType::SubpageOption, 3, defaultColors[22]));
        pages[index].options[4].addOption(Option("Rose", OptionType::SubpageOption, 3, defaultColors[23]));
        pages[index].options[4].addOption(Option("Rust", OptionType::SubpageOption, 3, defaultColors[24]));
        pages[index].options[4].addOption(Option("Coral", OptionType::SubpageOption, 3, defaultColors[25]));
        pages[index].options[4].addOption(Option("Peach", OptionType::SubpageOption, 3, defaultColors[26]));
        pages[index].options[4].addOption(Option("Gold", OptionType::SubpageOption, 3, defaultColors[27]));
        pages[index].options[4].addOption(Option("Yellow", OptionType::SubpageOption, 3, defaultColors[28]));
        pages[index].options[4].addOption(Option("Pale", OptionType::SubpageOption, 3, defaultColors[29]));
    }

    void init() {
        pages[0] = Page("Splitscreen", true);

        pages[0].addOption(Option("Remove Player", OptionType::Decrement));
        pages[0].addOption(Option("Display", OptionType::PointerDisplay));
        pages[0].addOption(Option("Add Player", OptionType::Increment));
        pages[0].addOption(Option("Use K/M for Player 1", OptionType::Boolean));

        for (int i = 1; i <= 4; i++) {
            initPlayer(i);
        }
    }
};