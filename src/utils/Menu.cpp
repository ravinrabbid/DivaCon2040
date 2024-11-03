#include "utils/Menu.h"

namespace Divacon::Utils {

const std::map<Menu::Page, const Menu::Descriptor> Menu::descriptors = {
    {Menu::Page::Main,                                              //
     {Menu::Descriptor::Type::Menu,                                 //
      "Settings",                                                   //
      {{"Mode", Menu::Descriptor::Action::GotoPageDeviceMode},      //
       {"Slider LED", Menu::Descriptor::Action::GotoPageLed},       //
       {"Reset", Menu::Descriptor::Action::GotoPageReset},          //
       {"USB Flash", Menu::Descriptor::Action::GotoPageBootsel}}}}, //

    {Menu::Page::Led,                                                       //
     {Menu::Descriptor::Type::Menu,                                         //
      "Slider LED",                                                         //
      {{"Brightness", Menu::Descriptor::Action::GotoPageLedBrightness},     //
       {"Anim Speed", Menu::Descriptor::Action::GotoPageLedAnimationSpeed}, //
       {"Idle Mode", Menu::Descriptor::Action::GotoPageLedIdleMode},        //
       {"Touch Mode", Menu::Descriptor::Action::GotoPageLedTouchedMode},    //
       {"Plyr Color", Menu::Descriptor::Action::GotoPagePlayerColor}}}},    //

    {Menu::Page::DeviceMode,                                 //
     {Menu::Descriptor::Type::Selection,                     //
      "Mode",                                                //
      {{"Swtch Diva", Menu::Descriptor::Action::SetUsbMode}, //
       {"Swtch Pro", Menu::Descriptor::Action::SetUsbMode},  //
       {"Dualshock3", Menu::Descriptor::Action::SetUsbMode}, //
       {"PS4 Diva", Menu::Descriptor::Action::SetUsbMode},   //
       {"Dualshock4", Menu::Descriptor::Action::SetUsbMode}, //
       {"Xbox 360", Menu::Descriptor::Action::SetUsbMode},   //
       {"PDL Arcade", Menu::Descriptor::Action::SetUsbMode}, //
       {"Keyboard", Menu::Descriptor::Action::SetUsbMode},   //
       {"MIDI", Menu::Descriptor::Action::SetUsbMode},       //
       {"Debug", Menu::Descriptor::Action::SetUsbMode}}}},   //

    {Menu::Page::LedBrightness,                             //
     {Menu::Descriptor::Type::Value,                        //
      "LED Brightness",                                     //
      {{"", Menu::Descriptor::Action::SetLedBrightness}}}}, //

    {Menu::Page::LedAnimationSpeed,                             //
     {Menu::Descriptor::Type::Value,                            //
      "LED Anim Speed",                                         //
      {{"", Menu::Descriptor::Action::SetLedAnimationSpeed}}}}, //

    {Menu::Page::LedIdleMode,                                       //
     {Menu::Descriptor::Type::Selection,                            //
      "LED Idle Mode",                                              //
      {{"Off", Menu::Descriptor::Action::SetLedIdleMode},           //
       {"Static", Menu::Descriptor::Action::SetLedIdleMode},        //
       {"Pulse", Menu::Descriptor::Action::SetLedIdleMode},         //
       {"Statc Rnbw", Menu::Descriptor::Action::SetLedIdleMode},    //
       {"Cycle Rnbw", Menu::Descriptor::Action::SetLedIdleMode}}}}, //

    {Menu::Page::LedTouchedMode,                                      //
     {Menu::Descriptor::Type::Selection,                              //
      "LED Touched Mode",                                             //
      {{"Off", Menu::Descriptor::Action::SetLedTouchedMode},          //
       {"Idle", Menu::Descriptor::Action::SetLedTouchedMode},         //
       {"Tchd Statc", Menu::Descriptor::Action::SetLedTouchedMode},   //
       {"Tchd Fade", Menu::Descriptor::Action::SetLedTouchedMode},    //
       {"Tchd Idle", Menu::Descriptor::Action::SetLedTouchedMode}}}}, //

    {Menu::Page::LedUsePlayerColor,                             //
     {Menu::Descriptor::Type::Toggle,                           //
      "Player Color (PS4)",                                     //
      {{"", Menu::Descriptor::Action::SetLedUsePlayerColor}}}}, //

    {Menu::Page::Reset,                               //
     {Menu::Descriptor::Type::Menu,                   //
      "Reset all Settings?",                          //
      {{"No", Menu::Descriptor::Action::GotoParent},  //
       {"Yes", Menu::Descriptor::Action::DoReset}}}}, //

    {Menu::Page::Bootsel, //
     {
         Menu::Descriptor::Type::Menu, //
         "Reboot to Flash Mode",       //
         {{"Reboot?", Menu::Descriptor::Action::DoRebootToBootsel}},
     }}, //

    {Menu::Page::BootselMsg,                           //
     {Menu::Descriptor::Type::RebootInfo,              //
      "Ready to Flash...",                             //
      {{"BOOTSEL", Menu::Descriptor::Action::None}}}}, //
};

Menu::Menu(std::shared_ptr<SettingsStore> settings_store)
    : m_store(settings_store), m_active(false), m_state_stack({{Page::Main, 0, 0}}) {};

void Menu::activate() {
    m_state_stack = std::stack<State>({{Page::Main, 0, 0}});
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

uint8_t Menu::getCurrentValue(Menu::Page page) {
    switch (page) {
    case Page::DeviceMode:
        return static_cast<uint8_t>(m_store->getUsbMode());
        break;
    case Page::LedBrightness:
        return m_store->getLedBrightness();
        break;
    case Page::LedAnimationSpeed:
        return m_store->getLedAnimationSpeed();
        break;
    case Page::LedIdleMode:
        return static_cast<uint8_t>(m_store->getLedIdleMode());
        break;
    case Page::LedTouchedMode:
        return static_cast<uint8_t>(m_store->getLedTouchedMode());
        break;
    case Page::LedUsePlayerColor:
        return m_store->getUsePlayerColor();
        break;
    case Page::Main:
    case Page::Led:
    case Page::Reset:
    case Page::Bootsel:
    case Page::BootselMsg:
        break;
    }

    return 0;
}

void Menu::gotoPage(Menu::Page page) {
    const auto current_value = getCurrentValue(page);

    m_state_stack.push({page, current_value, current_value});
}

void Menu::gotoParent(bool do_restore) {
    const auto current_state = m_state_stack.top();

    if (current_state.page == Page::Main) {
        m_active = false;
    }

    if (do_restore) {

        switch (current_state.page) {
        case Page::DeviceMode:
            m_store->setUsbMode(static_cast<usb_mode_t>(current_state.original_value));
            break;
        case Page::LedBrightness:
            m_store->setLedBrightness(current_state.original_value);
            break;
        case Page::LedAnimationSpeed:
            m_store->setLedAnimationSpeed(current_state.original_value);
            break;
        case Page::LedIdleMode:
            m_store->setLedIdleMode(
                static_cast<Peripherals::TouchSliderLeds::Config::IdleMode>(current_state.original_value));
            break;
        case Page::LedTouchedMode:
            m_store->setLedTouchedMode(
                static_cast<Peripherals::TouchSliderLeds::Config::TouchedMode>(current_state.original_value));
            break;
        case Page::LedUsePlayerColor:
            m_store->setUsePlayerColor(static_cast<bool>(current_state.original_value));
            break;
        case Page::Main:
        case Page::Led:
        case Page::Reset:
        case Page::Bootsel:
        case Page::BootselMsg:
            break;
        }
    }

    m_state_stack.pop();
}

void Menu::performAction(Descriptor::Action action, uint8_t value) {
    switch (action) {
    case Descriptor::Action::None:
        break;
    case Descriptor::Action::GotoParent:
        gotoParent(false);
        break;
    case Descriptor::Action::GotoPageDeviceMode:
        gotoPage(Page::DeviceMode);
        break;
    case Descriptor::Action::GotoPageLed:
        gotoPage(Page::Led);
        break;
    case Descriptor::Action::GotoPageLedBrightness:
        gotoPage(Page::LedBrightness);
        break;
    case Descriptor::Action::GotoPageLedAnimationSpeed:
        gotoPage(Page::LedAnimationSpeed);
        break;
    case Descriptor::Action::GotoPageLedIdleMode:
        gotoPage(Page::LedIdleMode);
        break;
    case Descriptor::Action::GotoPageLedTouchedMode:
        gotoPage(Page::LedTouchedMode);
        break;
    case Descriptor::Action::GotoPagePlayerColor:
        gotoPage(Page::LedUsePlayerColor);
        break;
    case Descriptor::Action::GotoPageReset:
        gotoPage(Page::Reset);
        break;
    case Descriptor::Action::GotoPageBootsel:
        gotoPage(Page::Bootsel);
        break;
    case Descriptor::Action::SetUsbMode:
        m_store->setUsbMode(static_cast<usb_mode_t>(value));
        break;
    case Descriptor::Action::SetLedIdleMode:
        m_store->setLedIdleMode(static_cast<Peripherals::TouchSliderLeds::Config::IdleMode>(value));
        break;
    case Descriptor::Action::SetLedTouchedMode:
        m_store->setLedTouchedMode(static_cast<Peripherals::TouchSliderLeds::Config::TouchedMode>(value));
        break;
    case Descriptor::Action::SetLedBrightness:
        m_store->setLedBrightness(value);
        break;
    case Descriptor::Action::SetLedAnimationSpeed:
        m_store->setLedAnimationSpeed(value);
        break;
    case Descriptor::Action::SetLedUsePlayerColor:
        m_store->setUsePlayerColor(static_cast<bool>(value));
        break;
    case Descriptor::Action::DoReset:
        m_store->reset();
        break;
    case Descriptor::Action::DoRebootToBootsel:
        m_store->scheduleReboot(true);
        gotoPage(Page::BootselMsg);
        break;
    }

    return;
}

void Menu::update(const InputState &input_state) {
    InputState::Buttons pressed = checkPressed(input_state);
    State &current_state = m_state_stack.top();

    auto descriptor_it = descriptors.find(current_state.page);
    if (descriptor_it == descriptors.end()) {
        assert(false);
        return;
    }

    if (descriptor_it->second.type == Descriptor::Type::RebootInfo) {
        m_active = false;
    } else if (pressed.north) { // Previous
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
            if (current_state.selected_value > 0) {
                current_state.selected_value--;
                performAction(descriptor_it->second.items.at(0).second, current_state.selected_value);
            }
            break;
        case Descriptor::Type::Toggle:
            current_state.selected_value = !current_state.selected_value;
            performAction(descriptor_it->second.items.at(0).second, current_state.selected_value);
            break;
        case Descriptor::Type::Selection:
            if (current_state.selected_value == 0) {
                current_state.selected_value = descriptor_it->second.items.size() - 1;
            } else {
                current_state.selected_value--;
            }
            performAction(descriptor_it->second.items.at(current_state.selected_value).second,
                          current_state.selected_value);
            break;
        case Descriptor::Type::Menu:
            if (current_state.selected_value == 0) {
                current_state.selected_value = descriptor_it->second.items.size() - 1;
            } else {
                current_state.selected_value--;
            }
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.west) { // Next
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
            if (current_state.selected_value < UINT8_MAX) {
                current_state.selected_value++;
                performAction(descriptor_it->second.items.at(0).second, current_state.selected_value);
            }
            break;
        case Descriptor::Type::Toggle:
            current_state.selected_value = !current_state.selected_value;
            performAction(descriptor_it->second.items.at(0).second, current_state.selected_value);
            break;
        case Descriptor::Type::Selection:
            if (current_state.selected_value == descriptor_it->second.items.size() - 1) {
                current_state.selected_value = 0;
            } else {
                current_state.selected_value++;
            }
            performAction(descriptor_it->second.items.at(current_state.selected_value).second,
                          current_state.selected_value);
            break;
        case Descriptor::Type::Menu:
            if (current_state.selected_value == descriptor_it->second.items.size() - 1) {
                current_state.selected_value = 0;
            } else {
                current_state.selected_value++;
            }
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.south) { // Back/Exit
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
        case Descriptor::Type::Toggle:
        case Descriptor::Type::Selection:
            gotoParent(true);
            break;
        case Descriptor::Type::Menu:
            gotoParent(false);
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.east) { // Select
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
        case Descriptor::Type::Toggle:
        case Descriptor::Type::Selection:
            gotoParent(false);
            break;
        case Descriptor::Type::Menu:
            performAction(descriptor_it->second.items.at(current_state.selected_value).second,
                          current_state.selected_value);
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    }
}

bool Menu::active() { return m_active; }

Menu::State Menu::getState() { return m_state_stack.top(); }

} // namespace Divacon::Utils
