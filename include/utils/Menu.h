#ifndef _UTILS_MENU_H_
#define _UTILS_MENU_H_

#include "utils/InputState.h"
#include "utils/SettingsStore.h"

#include <map>
#include <memory>
#include <stack>
#include <stddef.h>
#include <string>
#include <vector>

namespace Divacon::Utils {

class Menu {
  public:
    enum class Page {
        Main,
        DeviceMode,
        LedBrightness,
        Reset,
        Bootsel,
        BootselMsg,
    };

    struct State {
        Page page;
        uint8_t selection;
    };

    struct Descriptor {
        enum class Type {
            Root,
            Selection,
            Value,
            RebootInfo,
        };

        enum class Action {
            None,
            GotoParent,

            GotoPageDeviceMode,
            GotoPageLedBrightness,
            GotoPageReset,
            GotoPageBootsel,

            ChangeUsbModeSwitchDivacon,
            ChangeUsbModeSwitchHoripad,
            ChangeUsbModeDS3,
            ChangeUsbModePS4Divacon,
            ChangeUsbModeDS4,
            ChangeUsbModeXbox360,
            ChangeUsbModeMidi,
            ChangeUsbModeDebug,

            SetLedBrightness,

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

    uint8_t getCurrentSelection(Page page);
    void gotoPage(Page page);
    void gotoParent();
    void performSelectionAction(Descriptor::Action action);
    void performValueAction(Descriptor::Action action, uint8_t value);

  public:
    Menu(std::shared_ptr<SettingsStore> settings_store);

    void activate();
    void update(const InputState &input_state);
    bool active();
    State getState();
};
} // namespace Divacon::Utils

#endif // _UTILS_MENU_H_