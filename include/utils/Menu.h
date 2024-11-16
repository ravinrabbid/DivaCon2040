#ifndef _UTILS_MENU_H_
#define _UTILS_MENU_H_

#include "utils/InputState.h"
#include "utils/SettingsStore.h"

#include <map>
#include <memory>
#include <stack>
#include <string>
#include <vector>

namespace Divacon::Utils {

class Menu {
  public:
    enum class Page {
        Main,

        DeviceMode,
        Led,
        InputMirrorToDpad,
        Reset,
        Bootsel,

        LedBrightness,
        LedAnimationSpeed,
        LedIdleMode,
        LedIdleColor,
        LedTouchedMode,
        LedTouchedColor,
        LedEnablePlayerColor,
        LedEnablePdloaderSupport,

        LedIdleColorRed,
        LedIdleColorGreen,
        LedIdleColorBlue,

        LedTouchedColorRed,
        LedTouchedColorGreen,
        LedTouchedColorBlue,

        BootselMsg,
    };

    struct State {
        Page page;
        uint8_t selected_value;
        uint8_t original_value;
    };

    struct Descriptor {
        enum class Type {
            Menu,
            Selection,
            Value,
            Toggle,
            RebootInfo,
        };

        enum class Action {
            None,
            GotoParent,

            GotoPageDeviceMode,
            GotoPageLed,
            GotoPageLedBrightness,
            GotoPageLedAnimationSpeed,
            GotoPageLedIdleMode,
            GotoPageLedIdleColor,
            GotoPageLedTouchedMode,
            GotoPageLedTouchedColor,
            GotoPageLedEnablePlayerColor,
            GotoPageLedEnablePdloaderSupport,
            GotoPageInputMirrorToDpad,
            GotoPageReset,
            GotoPageBootsel,

            GotoPageLedIdleColorRed,
            GotoPageLedIdleColorGreen,
            GotoPageLedIdleColorBlue,

            GotoPageLedTouchedColorRed,
            GotoPageLedTouchedColorGreen,
            GotoPageLedTouchedColorBlue,

            SetUsbMode,

            SetLedBrightness,
            SetLedAnimationSpeed,
            SetLedIdleMode,
            SetLedTouchedMode,
            SetLedEnablePlayerColor,
            SetLedEnablePdloaderSupport,

            SetLedIdleColorRed,
            SetLedIdleColorGreen,
            SetLedIdleColorBlue,

            SetLedTouchedColorRed,
            SetLedTouchedColorGreen,
            SetLedTouchedColorBlue,

            SetInputMirrorToDpad,

            DoReset,
            DoRebootToBootsel,
        };

        Type type;
        std::string name;
        std::vector<std::pair<std::string, Action>> items;
    };

    const static std::map<Page, const Descriptor> descriptors;

  private:
    std::shared_ptr<SettingsStore> m_store;
    bool m_active;
    std::stack<State> m_state_stack;

    uint8_t getCurrentValue(Page page);
    void gotoPage(Page page);
    void gotoParent(bool do_restore);

    void performAction(Descriptor::Action action, uint8_t value);

  public:
    Menu(std::shared_ptr<SettingsStore> settings_store);

    void activate();
    void update(const InputState &input_state);
    bool active();
    State getState();
};
} // namespace Divacon::Utils

#endif // _UTILS_MENU_H_