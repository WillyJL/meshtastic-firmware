#if defined(M5STACK_CARDPUTER_ADV)

#include "CardputerAdvKeyboard.h"
#include "main.h"

#define _TCA8418_COLS 8
#define _TCA8418_ROWS 7
#define _TCA8418_NUM_KEYS 56

using Key = TCA8418KeyboardBase::TCA8418Key;

constexpr uint8_t modifierShiftKey = 7 - 1; // keynum -1
constexpr uint8_t modifierRightShift = 0b0001;

constexpr uint8_t modifierFnKey = 3 - 1;
constexpr uint8_t modifierFn = 0b0010;

constexpr uint8_t modifierCtrlKey = 4 - 1;

constexpr uint8_t modifierOptKey = 8 - 1;

constexpr uint8_t modifierAltKey = 12 - 1;

// Num chars per key, Modulus for rotating through characters
// https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1178/Sch_M5CardputerAdv_v1.0_2025_06_20_17_19_58_page_02.png
static const uint8_t CardputerAdvTapMod[_TCA8418_NUM_KEYS] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                                              3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                                              3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};

static const uint8_t CardputerAdvTapMap[_TCA8418_NUM_KEYS][3] = {
    {'`', '~', Key::ESC},
    {Key::TAB, 0x00, 0x00},
    {0x00, 0x00, 0x00}, // Fn
    {0x00, 0x00, 0x00}, // ctrl
    {'1', '!', 0x00},
    {'q', 'Q', Key::REBOOT},
    {0x00, 0x00, 0x00}, // shift
    {0x00, 0x00, 0x00}, // opt
    {'2', '@', 0x00},
    {'w', 'W', 0x00},
    {'a', 'A', 0x00},
    {0x00, 0x00, 0x00}, // alt
    {'3', '#', 0x00},
    {'e', 'E', 0x00},
    {'s', 'S', 0x00},
    {'z', 'Z', 0x00},
    {'4', '$', 0x00},
    {'r', 'R', 0x00},
    {'d', 'D', 0x00},
    {'x', 'X', 0x00},
    {'5', '%', 0x00},
    {'t', 'T', 0x00},
    {'f', 'F', 0x00},
    {'c', 'C', 0x00},
    {'6', '^', 0x00},
    {'y', 'Y', 0x00},
    {'g', 'G', Key::GPS_TOGGLE},
    {'v', 'V', 0x00},
    {'7', '&', 0x00},
    {'u', 'U', 0x00},
    {'h', 'H', 0x00},
    {'b', 'B', Key::BT_TOGGLE},
    {'8', '*', 0x00},
    {'i', 'I', 0x00},
    {'j', 'J', 0x00},
    {'n', 'N', 0x00},
    {'9', '(', 0x00},
    {'o', 'O', 0x00},
    {'k', 'K', 0x00},
    {'m', 'M', Key::MUTE_TOGGLE},
    {'0', ')', 0x00},
    {'p', 'P', Key::SEND_PING},
    {'l', 'L', 0x00},
    {',', '<', Key::LEFT},
    {'_', '-', 0x00},
    {'[', '{', 0x00},
    {';', ':', Key::UP},
    {'.', '>', Key::DOWN},
    {'=', '+', 0x00},
    {']', '}', 0x00},
    {'\'', '"', 0x00},
    {'/', '?', Key::RIGHT},
    {Key::BSP, 0x00, 0x00},
    {'\\', '|', 0x00},
    {Key::SELECT, 0x00, 0x00}, // Enter
    {' ', ' ', ' '},           // Space
};

static bool CardputerAdvHeldMap[_TCA8418_NUM_KEYS] = {};

CardputerAdvKeyboard::CardputerAdvKeyboard()
    : TCA8418KeyboardBase(_TCA8418_ROWS, _TCA8418_COLS), modifierFlag(0), pressedKeysCount(0), onlyOneModifierPressed(false),
      persistedPreviousModifier(false)
{
    reset();
}

void CardputerAdvKeyboard::reset(void)
{
    TCA8418KeyboardBase::reset();
}

int8_t CardputerAdvKeyboard::keyToIndex(uint8_t key)
{
    uint8_t key_index = 0;
    int row = (key - 1) / 10;
    int col = (key - 1) % 10;

    if (row >= _TCA8418_ROWS || col >= _TCA8418_COLS) {
        return -1; // Invalid key
    }

    key_index = row * _TCA8418_COLS + col;
    return key_index;
}

void CardputerAdvKeyboard::pressed(uint8_t key)
{
    int8_t key_index = keyToIndex(key);
    if (key_index < 0)
        return;

    if (CardputerAdvHeldMap[key_index]) {
        return;
    }

    CardputerAdvHeldMap[key_index] = true;
    pressedKeysCount++;

    uint8_t key_modifier = keyToModifierFlag(key_index);
    if (key_modifier && pressedKeysCount == 1) {
        onlyOneModifierPressed = true;
    } else {
        onlyOneModifierPressed = false;
    }
    modifierFlag |= key_modifier;
}

void CardputerAdvKeyboard::released(uint8_t key)
{
    int8_t key_index = keyToIndex(key);
    if (key_index < 0)
        return;

    if (!CardputerAdvHeldMap[key_index]) {
        return;
    }

    queueEvent(CardputerAdvTapMap[key_index][modifierFlag % CardputerAdvTapMod[key_index]]);

    CardputerAdvHeldMap[key_index] = false;
    pressedKeysCount--;

    if (onlyOneModifierPressed) {
        onlyOneModifierPressed = false;
        if (persistedPreviousModifier) {
            modifierFlag = 0;
        }
        persistedPreviousModifier = !persistedPreviousModifier;
    } else if (persistedPreviousModifier && pressedKeysCount == 0) {
        modifierFlag = 0;
        persistedPreviousModifier = false;
    } else {
        modifierFlag &= ~keyToModifierFlag(key_index);
    }
}

uint8_t CardputerAdvKeyboard::keyToModifierFlag(uint8_t key)
{
    if (key == modifierShiftKey) {
        return modifierRightShift;
    } else if (key == modifierFnKey) {
        return modifierFn;
    } else if (key == modifierCtrlKey) {
        // return modifierCtrl;
    } else if (key == modifierOptKey) {
        // return modifierOpt;
    } else if (key == modifierAltKey) {
        // return modifierAlt;
    }
    return 0;
}

bool CardputerAdvKeyboard::isModifierKey(uint8_t key)
{
    return keyToModifierFlag(key) != 0;
}

#endif