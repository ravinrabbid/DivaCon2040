#ifndef _UTILS_MENU_H_
#define _UTILS_MENU_H_

#include "utils/InputState.h"
#include "utils/SettingsStore.h"

#include <map>
#include <memory>
#include <stddef.h>
#include <string>
#include <vector>

namespace Divacon::Utils {

class Menu {
  public:
    enum class Page {
        None,
        Main,
        DeviceMode,
        LedBrightness,
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

            GotoPageDeviceMode,
            GotoPageLedBrightness,
            GotoPageBootsel,

            ChangeUsbModeSwitchDivacon,
            ChangeUsbModeSwitchHoripad,
            ChangeUsbModeDS3,
            ChangeUsbModePS4Divacon,
            ChangeUsbModeDS4,
            ChangeUsbModeXbox360,
            ChangeUsbModeDebug,

            SetLedBrightness,

            DoRebootToBootsel,
        };

        Type type;
        std::string name;
        std::vector<std::pair<std::string, Action>> items;
        Page parent;
    };

    const static std::map<Page, const Descriptor> descriptors;

  private:
    std::shared_ptr<SettingsStore> m_store;
    bool m_active;
    State m_state;

    uint8_t getCurrentSelection(Page page);
    void gotoPage(Page page);
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