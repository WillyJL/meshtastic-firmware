#include "TCA8418KeyboardBase.h"

class CardputerAdvKeyboard : public TCA8418KeyboardBase
{
  public:
    CardputerAdvKeyboard();
    void reset(void);
    virtual ~CardputerAdvKeyboard() {}

  protected:
    void pressed(uint8_t key) override;
    void released(uint8_t key) override;
    int8_t keyToIndex(uint8_t key);

    uint8_t keyToModifierFlag(uint8_t key);
    bool isModifierKey(uint8_t key);

  private:
    uint8_t modifierFlag; // Flag to indicate if a modifier key is pressed
    uint8_t pressedKeysCount;
    bool onlyOneModifierPressed;
    bool persistedPreviousModifier;
};
