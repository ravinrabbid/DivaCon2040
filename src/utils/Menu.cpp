#include "utils/Menu.h"

#include "pico/bootrom.h"

namespace Divacon::Utils {

const std::map<Menu::Page, const Menu::Descriptor> Menu::descriptors = {
    {Menu::Page::Main,                                                  //
     {Menu::Descriptor::Type::Root,                                     //
      "Settings",                                                       //
      {{"USB Proto", Menu::Descriptor::Action::GotoPageUsbMode},        //
       {"Sldr Mode", Menu::Descriptor::Action::GotoPageSliderMode},     //
       {"Brightness", Menu::Descriptor::Action::GotoPageLedBrightness}, //
       {"BOOTSEL", Menu::Descriptor::Action::GotoPageBootsel}},         //
      Menu::Page::None}},                                               //

    {Menu::Page::UsbMode,                                         //
     {Menu::Descriptor::Type::Selection,                          //
      "USB Mode",                                                 //
      {{"DInput", Menu::Descriptor::Action::ChangeUsbModeDInput}, //
       {"Switch", Menu::Descriptor::Action::ChangeUsbModeSwitch}, //
       {"XInput", Menu::Descriptor::Action::ChangeUsbModeXInput}, //
       {"Debug", Menu::Descriptor::Action::ChangeUsbModeDebug}},  //
      Menu::Page::Main}},                                         //

    {Menu::Page::SliderMode,                                         //
     {Menu::Descriptor::Type::Selection,                             //
      "Slider Mode",                                                 //
      {{"Arcade", Menu::Descriptor::Action::ChangeSliderModeArcade}, //
       {"Stick", Menu::Descriptor::Action::ChangeSliderModeStick}},  //
      Menu::Page::Main}},                                            //

    {Menu::Page::LedBrightness,                           //
     {Menu::Descriptor::Type::Value,                      //
      "LED Brightness",                                   //
      {{"", Menu::Descriptor::Action::SetLedBrightness}}, //
      Menu::Page::Main}},                                 //

    {Menu::Page::Bootsel,                                         //
     {Menu::Descriptor::Type::Selection,                          //
      "Reboot to BOOTSEL",                                        //
      {{"Reboot?", Menu::Descriptor::Action::DoRebootToBootsel}}, //
      Menu::Page::Main}},                                         //
};

Menu::Menu(std::shared_ptr<SettingsStore> settings_store)
    : m_store(settings_store), m_active(false), m_state({Page::Main, 0}){};

void Menu::activate() {
    m_state = {Page::Main, 0};
    m_active = true;
}

static InputState::Buttons checkPressed(const InputState &input_state) {
    struct ButtonState {
        enum State {
            Idle,
            RepeatDelay,
            Repeat,
        };
        State state;
        uint32_t pressed_since;
        uint32_t last_repeat;
    };

    static const uint32_t repeat_delay = 1000;
    static const uint32_t repeat_interval = 20;

    static ButtonState state_north = {ButtonState::State::Idle, 0, 0};
    static ButtonState state_east = {ButtonState::State::Idle, 0, 0};
    static ButtonState state_south = {ButtonState::State::Idle, 0, 0};
    static ButtonState state_west = {ButtonState::State::Idle, 0, 0};

    InputState::Buttons result{false, false, false, false, false, false, false,
                               false, false, false, false, false, false};

    auto handle_button = [](ButtonState &button_state, bool input_state) {
        bool result = false;
        if (input_state) {
            uint32_t now = to_ms_since_boot(get_absolute_time());
            switch (button_state.state) {
            case ButtonState::State::Idle:
                result = true;
                button_state.state = ButtonState::State::RepeatDelay;
                button_state.pressed_since = now;
                break;
            case ButtonState::State::RepeatDelay:
                if ((now - button_state.pressed_since) > repeat_delay) {
                    result = true;
                    button_state.state = ButtonState::State::Repeat;
                    button_state.last_repeat = now;
                } else {
                    result = false;
                }
                break;
            case ButtonState::State::Repeat:
                if ((now - button_state.last_repeat) > repeat_interval) {
                    result = true;
                    button_state.last_repeat = now;
                } else {
                    result = false;
                }
                break;
            }
        } else {
            result = false;
            button_state.state = ButtonState::State::Idle;
        }

        return result;
    };

    result.north = handle_button(state_north, input_state.buttons.north);
    result.east = handle_button(state_east, input_state.buttons.east);
    result.south = handle_button(state_south, input_state.buttons.south);
    result.west = handle_button(state_west, input_state.buttons.west);

    return result;
}

uint8_t Menu::getCurrentSelection(Menu::Page page) {
    switch (page) {
    case Page::UsbMode:
        return static_cast<uint8_t>(m_store->getUsbMode());
        break;
    case Page::SliderMode:
        return static_cast<uint8_t>(m_store->getSliderMode());
        break;
    case Page::LedBrightness:
        return m_store->getLedBrightness();
        break;
    case Page::Main:
    case Page::Bootsel:
    case Page::None:
        break;
    }

    return 0;
}

void Menu::gotoPage(Menu::Page page) {
    m_state.page = page;
    m_state.selection = getCurrentSelection(page);
}

void Menu::performSelectionAction(Menu::Descriptor::Action action) {
    auto descriptor_it = descriptors.find(m_state.page);
    if (descriptor_it == descriptors.end()) {
        assert(false);
        return;
    }

    switch (action) {
    case Descriptor::Action::GotoPageUsbMode:
        gotoPage(Page::UsbMode);
        break;
    case Descriptor::Action::GotoPageSliderMode:
        gotoPage(Page::SliderMode);
        break;
    case Descriptor::Action::GotoPageLedBrightness:
        gotoPage(Page::LedBrightness);
        break;
    case Descriptor::Action::GotoPageBootsel:
        gotoPage(Page::Bootsel);
        break;
    case Descriptor::Action::ChangeUsbModeDInput:
        m_store->setUsbMode(USB_MODE_DIRECTINPUT);
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::ChangeUsbModeSwitch:
        m_store->setUsbMode(USB_MODE_SWITCH);
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::ChangeUsbModeXInput:
        m_store->setUsbMode(USB_MODE_XINPUT);
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::ChangeUsbModeDebug:
        m_store->setUsbMode(USB_MODE_DEBUG);
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::ChangeSliderModeArcade:
        m_store->setSliderMode(Peripherals::TouchSlider::Config::Mode::ARCADE);
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::ChangeSliderModeStick:
        m_store->setSliderMode(Peripherals::TouchSlider::Config::Mode::STICK);
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::SetLedBrightness:
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::DoRebootToBootsel:
        // TODO show reboot message
        reset_usb_boot(0, 0);
        break;
    default:
        break;
    }
}

void Menu::performValueAction(Menu::Descriptor::Action action, uint8_t value) {
    auto descriptor_it = descriptors.find(m_state.page);
    if (descriptor_it == descriptors.end()) {
        assert(false);
        return;
    }

    switch (action) {
    case Descriptor::Action::SetLedBrightness:
        m_store->setLedBrightness(value);
        break;
    default:
        break;
    }
}

void Menu::update(const InputState &input_state) {
    InputState::Buttons pressed = checkPressed(input_state);

    auto descriptor_it = descriptors.find(m_state.page);
    if (descriptor_it == descriptors.end()) {
        assert(false);
        return;
    }

    if (pressed.north) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
            if (m_state.selection > 0) {
                m_state.selection--;
                performValueAction(descriptor_it->second.items.at(0).second, m_state.selection);
            }
            break;
        case Descriptor::Type::Selection:
        case Descriptor::Type::Root:
            if (m_state.selection == 0) {
                m_state.selection = descriptor_it->second.items.size() - 1;
            } else {
                m_state.selection--;
            }
            break;
        }
    } else if (pressed.west) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
            if (m_state.selection < UINT8_MAX) {
                m_state.selection++;
                performValueAction(descriptor_it->second.items.at(0).second, m_state.selection);
            }
            break;
        case Descriptor::Type::Selection:
        case Descriptor::Type::Root:
            if (m_state.selection == descriptor_it->second.items.size() - 1) {
                m_state.selection = 0;
            } else {
                m_state.selection++;
            }
            break;
        }
    } else if (pressed.south) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
        case Descriptor::Type::Selection:
            gotoPage(descriptor_it->second.parent);
            break;
        case Descriptor::Type::Root:
            m_active = false;
            break;
        }
    } else if (pressed.east) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
            performSelectionAction(descriptor_it->second.items.at(0).second);
            break;
        case Descriptor::Type::Selection:
        case Descriptor::Type::Root:
            performSelectionAction(descriptor_it->second.items.at(m_state.selection).second);
            break;
        }
    }
}

bool Menu::active() { return m_active; }

Menu::State Menu::getState() { return m_state; }

} // namespace Divacon::Utils
