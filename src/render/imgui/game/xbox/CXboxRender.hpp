#pragma once

#include "CStateMachine.h"

#include "draw/CXboxRect.hpp"
#include "draw/CXboxButton.hpp"
#include "draw/CXboxPage.hpp"

#include "assets/colors.h"
#include "assets/mccIcon.h"

#include <algorithm>
#include <optional>
#include <cmath>

void renderMenu(
    int WINDOW_WIDTH,
    int WINDOW_HEIGHT,
    State state,
    ImFont* font
) {
    float alpha = std::min(state.time / state.duration, 1.0f);

    float menuScale = 0.0f;
    int menuWidth = 0;
    int menuHeight = 0;
    int menuPosX = 0;
    int menuPosY = 0;

    int buttonSize = 50;
    float buttonOffset = 0.0f;

    int iconSize = 45;
    int pageCount = State().menu.MAX_PAGES;

    ImU8 globalAlpha = 255;

    // -------------------------
    // MENU SCALE / ANIMATION
    // -------------------------
    
    if (state.phase == Phase::Opening) {
        menuScale = 0.45f * alpha;
        menuWidth = WINDOW_WIDTH * menuScale;
        menuHeight = (alpha > 0.5f)
            ? WINDOW_HEIGHT * menuScale
            : WINDOW_HEIGHT * menuScale * 0.5f;
    }
    else if (state.phase == Phase::Closing) {
        menuScale = 0.45f * (1.0f - alpha);
        menuWidth = WINDOW_WIDTH * menuScale;
        menuHeight = (alpha < 0.5f)
            ? WINDOW_HEIGHT * menuScale
            : WINDOW_HEIGHT * menuScale * 0.5f;
    }
    else {
        menuScale = 0.45f;
        menuWidth = WINDOW_WIDTH * menuScale;
        menuHeight = WINDOW_HEIGHT * menuScale;
    }

    menuPosX = (WINDOW_WIDTH - menuWidth) / 2;
    menuPosY = (WINDOW_HEIGHT - menuHeight) / 2;

    // -------------------------
    // GLOBAL ALPHA
    // -------------------------
    if (state.phase == Phase::FadeIn) {
        globalAlpha = static_cast<ImU8>(255 * alpha);
    }
    else if (
        state.phase == Phase::ShiftRight ||
        state.phase == Phase::ShiftLeft ||
        state.phase == Phase::ShiftIn ||
        state.phase == Phase::ShiftOut
    ) {
        globalAlpha = static_cast<ImU8>(255 * (1.0f - alpha));
    }

    // -------------------------
    // BUTTON OFFSET (SUB MENU)
    // -------------------------
    if (state.phase == Phase::ShiftUp) {
        buttonOffset = -(buttonSize * state.subOptionWindow[0]) + (buttonSize * alpha);
    }
    else if (state.phase == Phase::ShiftDown) {
        buttonOffset = -(buttonSize * state.subOptionWindow[0]) - (buttonSize * alpha);
    }
    else if (
        state.phase == Phase::InIdle ||
        state.phase == Phase::InFadeIn
    ) {
        buttonOffset = -(buttonSize * state.subOptionWindow[0]);
    }

    // -------------------------
    // PAGE TRANSITION OFFSETS
    // -------------------------
    auto pageOffset = [&](bool rightSide) -> int {
        int updateSize = 50;

        if (state.phase == Phase::ShiftRight)
            return rightSide ? (-updateSize * alpha - 50) : (-updateSize * alpha + 50);

        if (state.phase == Phase::ShiftLeft)
            return rightSide ? (updateSize * alpha - 50) : (updateSize * alpha + 50);

        if (state.phase == Phase::ShiftIn)
            return rightSide ? (-updateSize * alpha * (alpha * 4)) : (updateSize * alpha * (alpha * 4));

        if (state.phase == Phase::ShiftOut)
            return rightSide ? (updateSize * alpha - 50) : (-updateSize * alpha + 50);

        return 0;
    };

    // -------------------------
    // LEFT PAGES
    // -------------------------
    if (state.phase != Phase::Opening && state.phase != Phase::Closing) {
        int offset = pageOffset(false);
        int prefixSize = -50;

        int extra = (state.phase == Phase::ShiftRight || state.phase == Phase::ShiftLeft) ? 1 : 0;

        for (int i = state.pageIndex - 1 + extra; i >= 0; --i) {
            drawPage(
                offset + menuPosX + prefixSize,
                menuPosY,
                menuWidth,
                menuHeight,
                state.menu.pages[i].label.data(),
                false,
                255,
                font
            );
            prefixSize -= 50;
        }
    }

    // -------------------------
    // ICON + TITLE
    // -------------------------
    drawText(
        menuPosX - (menuWidth / 2) + 10,
        menuPosY - 190,
        menuWidth,
        menuHeight,
        font,
        25.0f,
        IM_COL32(255, 255, 255, globalAlpha),
        "MCC Guide"
    );

    // NOTE: icon rendering depends on your ImGui texture system.
    // If you have ImTextureID:
    // drawIcon(...) should be replaced with ImGui::Image(...)

    // -------------------------
    // CURRENT PAGE (MAIN)
    // -------------------------
    const auto& page = state.menu.pages[state.pageIndex];

    if (
        state.phase == Phase::Idle ||
        state.phase == Phase::FadeIn ||
        state.phase == Phase::ShiftRight ||
        state.phase == Phase::ShiftLeft ||
        state.phase == Phase::ShiftIn
    ) {
        drawPage(
            menuPosX,
            menuPosY,
            menuWidth,
            menuHeight,
            page.label.data(),
            true,
            globalAlpha,
            font
        );
    }

    // -------------------------
    // RIGHT PAGES
    // -------------------------
    if (state.phase != Phase::Opening &&
        state.phase != Phase::Closing &&
        state.pageIndex <= pageCount - 1) {

        int offset = pageOffset(true);
        int suffixSize = menuWidth + 50;

        int extra = (state.phase == Phase::ShiftRight || state.phase == Phase::ShiftLeft) ? 1 : 0;

        for (int i = state.pageIndex + 1 - extra; i < pageCount; ++i) {
            drawPage(
                offset + menuPosX + suffixSize,
                menuPosY,
                menuWidth,
                menuHeight,
                state.menu.pages[i].label.data(),
                false,
                255,
                font
            );
            suffixSize += 50;
        }
    }

    // -------------------------
    // BACKGROUND
    // -------------------------
    drawGradientRect(
        menuPosX,
        menuPosY,
        menuWidth,
        menuHeight,
        IM_COL32(255, 255, 255, globalAlpha),
        IM_COL32(255, 255, 255, globalAlpha),
        true
    );

    // -------------------------
    // OPTIONS (MAIN PAGE)
    // -------------------------
    if (
        state.phase == Phase::Idle ||
        state.phase == Phase::FadeIn ||
        state.phase == Phase::ShiftRight ||
        state.phase == Phase::ShiftLeft ||
        state.phase == Phase::ShiftIn
    ) {
        float buttonCount = 0.0f;

        for (int i = 0; i < page.options.size(); i++) {
            const auto& opt = page.options[i];
            OptionType type = opt.type;

            int yBase = menuPosY + buttonCount * buttonSize;

            if (type == OptionType::Increment ||
                type == OptionType::Decrement ||
                type == OptionType::PointerDisplay) {

                drawButton(
                    menuPosX + i * (menuWidth / 3),
                    menuPosY,
                    menuWidth,
                    menuHeight,
                    font,
                    state.optionIndex == i,
                    globalAlpha,
                    std::to_string(state.menuState.playerCount).c_str(),
                    type,
                    0
                );
                buttonCount += 0.33f;
                continue;
            }

            if (type == OptionType::Boolean) {
                drawButton(
                    menuPosX,
                    yBase,
                    menuWidth,
                    menuHeight,
                    font,
                    state.optionIndex == i,
                    globalAlpha,
                    opt.label.c_str(),
                    type,
                    state.menuState.useKM ? 1 : 0
                );
                buttonCount++;
                continue;
            }

            if (type == OptionType::TeamToggle) {
                drawButton(
                    menuPosX,
                    yBase,
                    menuWidth,
                    menuHeight,
                    font,
                    state.optionIndex == i,
                    globalAlpha,
                    opt.label.c_str(),
                    type,
                    state.menuState.teamIndex[state.pageIndex - 1]
                );
                buttonCount++;
                continue;
            }

            if (type == OptionType::Subpage) {
                drawButton(
                    menuPosX,
                    yBase,
                    menuWidth,
                    menuHeight,
                    font,
                    state.optionIndex == i,
                    globalAlpha,
                    opt.label.c_str(),
                    type,
                    0
                );
                buttonCount++;
                continue;
            }

            drawButton(
                menuPosX,
                yBase,
                menuWidth,
                menuHeight,
                font,
                state.optionIndex == i,
                globalAlpha,
                opt.label.c_str(),
                type,
                0
            );

            buttonCount++;
        }
    }

    // -------------------------
    // SUB OPTIONS (SUB MENU STATE)
    // -------------------------
    if (
        state.phase == Phase::InIdle ||
        state.phase == Phase::InFadeIn ||
        state.phase == Phase::ShiftOut ||
        state.phase == Phase::ShiftUp ||
        state.phase == Phase::ShiftDown
    ) {
        float buttonCount = 0.0f;

        const auto& subOpts =
            page.options[state.optionIndex].subOptions;

        for (int i = 0; i < subOpts.size(); i++) {
            const auto& opt = subOpts[i];
            OptionType type = state.menu.pages[state.pageIndex].options[state.optionIndex].subOptions[i].type;
            drawButton(
                menuPosX,
                menuPosY + buttonCount * buttonSize + buttonOffset,
                menuWidth,
                menuHeight,
                font,
                state.subOptionIndex == i,
                globalAlpha,
                opt.label.c_str(),
                type,
                0,
                state.menu.pages[state.pageIndex].options[state.optionIndex].subOptions[i].colorValue
            );

            buttonCount++;
        }
    }
}