#include "utils/Menu.h"

namespace Divacon::Utils {

const std::map<Menu::Page, const Menu::Descriptor> Menu::descriptors = {
    {Menu::Page::Main,                                              //
     {Menu::Descriptor::Type::Root,                                 //
      "Settings",                                                   //
      {{"Mode", Menu::Descriptor::Action::GotoPageDeviceMode},      //
       {"Slider LED", Menu::Descriptor::Action::GotoPageLed},       //
       {"Reset", Menu::Descriptor::Action::GotoPageReset},          //
       {"USB Flash", Menu::Descriptor::Action::GotoPageBootsel}}}}, //

    {Menu::Page::Led,                                                       //
     {Menu::Descriptor::Type::Selection,                                    //
      "Slider LED",                                                         //
      {{"Brightness", Menu::Descriptor::Action::GotoPageLedBrightness},     //
       {"Anim Speed", Menu::Descriptor::Action::GotoPageLedAnimationSpeed}, //
       {"Idle Mode", Menu::Descriptor::Action::GotoPageLedIdleMode},        //
       {"Touch Mode", Menu::Descriptor::Action::GotoPageLedTouchedMode},    //
       {"Plyr Color", Menu::Descriptor::Action::GotoPagePlayerColor}}}},    //

    {Menu::Page::DeviceMode,                                                 //
     {Menu::Descriptor::Type::Selection,                                     //
      "Mode",                                                                //
      {{"Swtch Diva", Menu::Descriptor::Action::ChangeUsbModeSwitchDivacon}, //
       {"Swtch Pro", Menu::Descriptor::Action::ChangeUsbModeSwitchHoripad},  //
       {"Dualshock3", Menu::Descriptor::Action::ChangeUsbModeDS3},           //
       {"PS4 Diva", Menu::Descriptor::Action::ChangeUsbModePS4Divacon},      //
       {"Dualshock4", Menu::Descriptor::Action::ChangeUsbModeDS4},           //
       {"Xbox 360", Menu::Descriptor::Action::ChangeUsbModeXbox360},         //
       {"PDL Arcade", Menu::Descriptor::Action::ChangeUsbModePDLoader},      //
       {"Keyboard", Menu::Descriptor::Action::ChangeUsbModeKeyboard},        //
       {"MIDI", Menu::Descriptor::Action::ChangeUsbModeMidi},                //
       {"Debug", Menu::Descriptor::Action::ChangeUsbModeDebug}}}},           //

    {Menu::Page::LedBrightness,                             //
     {Menu::Descriptor::Type::Value,                        //
      "LED Brightness",                                     //
      {{"", Menu::Descriptor::Action::SetLedBrightness}}}}, //

    {Menu::Page::LedAnimationSpeed,                             //
     {Menu::Descriptor::Type::Value,                            //
      "LED Anim Speed",                                         //
      {{"", Menu::Descriptor::Action::SetLedAnimationSpeed}}}}, //

    {Menu::Page::LedIdleMode,                                                      //
     {Menu::Descriptor::Type::Selection,                                           //
      "LED Idle Mode",                                                             //
      {{"Off", Menu::Descriptor::Action::ChangeLedIdleModeOff},                    //
       {"Static", Menu::Descriptor::Action::ChangeLedIdleModeStatic},              //
       {"Pulse", Menu::Descriptor::Action::ChangeLedIdleModePulse},                //
       {"Statc Rnbw", Menu::Descriptor::Action::ChangeLedIdleModeRainbowStatic},   //
       {"Cycle Rnbw", Menu::Descriptor::Action::ChangeLedIdleModeRainbowCycle}}}}, //

    {Menu::Page::LedTouchedMode,                                                    //
     {Menu::Descriptor::Type::Selection,                                            //
      "LED Touched Mode",                                                           //
      {{"Off", Menu::Descriptor::Action::ChangeLedTouchedModeOff},                  //
       {"Idle", Menu::Descriptor::Action::ChangeLedTouchedModeIdle},                //
       {"Tchd Statc", Menu::Descriptor::Action::ChangeLedTouchedModeTouched},       //
       {"Tchd Fade", Menu::Descriptor::Action::ChangeLedTouchedModeTouchedFade},    //
       {"Tchd Idle", Menu::Descriptor::Action::ChangeLedTouchedModeTouchedIdle}}}}, //
    {Menu::Page::UsePlayerColor,                             //
     {Menu::Descriptor::Type::Toggle,                        //
      "Player Color (PS4)",                                  //
      {{"", Menu::Descriptor::Action::SetUsePlayerColor}}}}, //

    {Menu::Page::Reset,                               //
     {Menu::Descriptor::Type::Selection,              //
      "Reset all Settings?",                          //
      {{"No", Menu::Descriptor::Action::GotoParent},  //
       {"Yes", Menu::Descriptor::Action::DoReset}}}}, //

    {Menu::Page::Bootsel, //
     {
         Menu::Descriptor::Type::Selection, //
         "Reboot to Flash Mode",            //
         {{"Reboot?", Menu::Descriptor::Action::DoRebootToBootsel}},
     }}, //

    {Menu::Page::BootselMsg,                           //
     {Menu::Descriptor::Type::RebootInfo,              //
      "Ready to Flash...",                             //
      {{"BOOTSEL", Menu::Descriptor::Action::None}}}}, //
};

Menu::Menu(std::shared_ptr<SettingsStore> settings_store)
    : m_store(settings_store), m_active(false), m_state_stack({{Page::Main, 0}}) {};

void Menu::activate() {
    m_state_stack = std::stack<State>({{Page::Main, 0}});
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
    case Page::UsePlayerColor:
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

void Menu::gotoPage(Menu::Page page) { m_state_stack.push({page, getCurrentSelection(page)}); }

void Menu::gotoParent() { m_state_stack.pop(); }

void Menu::performSelectionAction(Menu::Descriptor::Action action) {
    auto descriptor_it = descriptors.find(m_state_stack.top().page);
    if (descriptor_it == descriptors.end()) {
        assert(false);
        return;
    }

    switch (action) {
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
        gotoPage(Page::UsePlayerColor);
        break;
    case Descriptor::Action::GotoPageReset:
        gotoPage(Page::Reset);
        break;
    case Descriptor::Action::GotoPageBootsel:
        gotoPage(Page::Bootsel);
        break;
    case Descriptor::Action::ChangeUsbModeSwitchDivacon:
        m_store->setUsbMode(USB_MODE_SWITCH_DIVACON);
        m_store->setSliderMode(Peripherals::TouchSlider::Config::Mode::ARCADE);
        gotoParent();
        break;
    case Descriptor::Action::ChangeUsbModeSwitchHoripad:
        m_store->setUsbMode(USB_MODE_SWITCH_HORIPAD);
        m_store->setSliderMode(Peripherals::TouchSlider::Config::Mode::STICK);
        gotoParent();
        break;
    case Descriptor::Action::ChangeUsbModeDS3:
        m_store->setUsbMode(USB_MODE_DUALSHOCK3);
        m_store->setSliderMode(Peripherals::TouchSlider::Config::Mode::STICK);
        gotoParent();
        break;
    case Descriptor::Action::ChangeUsbModePS4Divacon:
        m_store->setUsbMode(USB_MODE_PS4_DIVACON);
        m_store->setSliderMode(Peripherals::TouchSlider::Config::Mode::ARCADE);
        gotoParent();
        break;
    case Descriptor::Action::ChangeUsbModeDS4:
        m_store->setUsbMode(USB_MODE_DUALSHOCK4);
        m_store->setSliderMode(Peripherals::TouchSlider::Config::Mode::STICK);
        gotoParent();
        break;
    case Descriptor::Action::ChangeUsbModeXbox360:
        m_store->setUsbMode(USB_MODE_XBOX360);
        m_store->setSliderMode(Peripherals::TouchSlider::Config::Mode::STICK);
        gotoParent();
        break;
    case Descriptor::Action::ChangeUsbModePDLoader:
        m_store->setUsbMode(USB_MODE_PDLOADER);
        m_store->setSliderMode(Peripherals::TouchSlider::Config::Mode::ARCADE);
        gotoParent();
        break;
    case Descriptor::Action::ChangeUsbModeKeyboard:
        m_store->setUsbMode(USB_MODE_KEYBOARD);
        m_store->setSliderMode(Peripherals::TouchSlider::Config::Mode::STICK);
        gotoParent();
        break;
    case Descriptor::Action::ChangeUsbModeMidi:
        m_store->setUsbMode(USB_MODE_MIDI);
        m_store->setSliderMode(Peripherals::TouchSlider::Config::Mode::ARCADE);
        gotoParent();
        break;
    case Descriptor::Action::ChangeUsbModeDebug:
        m_store->setUsbMode(USB_MODE_DEBUG);
        m_store->setSliderMode(Peripherals::TouchSlider::Config::Mode::ARCADE);
        gotoParent();
        break;
    case Descriptor::Action::ChangeLedIdleModeOff:
        m_store->setLedIdleMode(Peripherals::TouchSliderLeds::Config::IdleMode::Off);
        gotoParent();
        break;
    case Descriptor::Action::ChangeLedIdleModeStatic:
        m_store->setLedIdleMode(Peripherals::TouchSliderLeds::Config::IdleMode::Static);
        gotoParent();
        break;
    case Descriptor::Action::ChangeLedIdleModePulse:
        m_store->setLedIdleMode(Peripherals::TouchSliderLeds::Config::IdleMode::Pulse);
        gotoParent();
        break;
    case Descriptor::Action::ChangeLedIdleModeRainbowStatic:
        m_store->setLedIdleMode(Peripherals::TouchSliderLeds::Config::IdleMode::RainbowStatic);
        gotoParent();
        break;
    case Descriptor::Action::ChangeLedIdleModeRainbowCycle:
        m_store->setLedIdleMode(Peripherals::TouchSliderLeds::Config::IdleMode::RainbowCycle);
        gotoParent();
        break;
    case Descriptor::Action::ChangeLedTouchedModeOff:
        m_store->setLedTouchedMode(Peripherals::TouchSliderLeds::Config::TouchedMode::Off);
        gotoParent();
        break;
    case Descriptor::Action::ChangeLedTouchedModeIdle:
        m_store->setLedTouchedMode(Peripherals::TouchSliderLeds::Config::TouchedMode::Idle);
        gotoParent();
        break;
    case Descriptor::Action::ChangeLedTouchedModeTouched:
        m_store->setLedTouchedMode(Peripherals::TouchSliderLeds::Config::TouchedMode::Touched);
        gotoParent();
        break;
    case Descriptor::Action::ChangeLedTouchedModeTouchedFade:
        m_store->setLedTouchedMode(Peripherals::TouchSliderLeds::Config::TouchedMode::TouchedFade);
        gotoParent();
        break;
    case Descriptor::Action::ChangeLedTouchedModeTouchedIdle:
        m_store->setLedTouchedMode(Peripherals::TouchSliderLeds::Config::TouchedMode::TouchedIdle);
        gotoParent();
        break;
    case Descriptor::Action::SetLedBrightness:
        gotoParent();
        break;
    case Descriptor::Action::SetLedAnimationSpeed:
        gotoParent();
        break;
    case Descriptor::Action::SetUsePlayerColor:
        gotoParent();
        break;
    case Descriptor::Action::DoReset:
        m_store->reset();
        break;
    case Descriptor::Action::DoRebootToBootsel:
        m_store->scheduleReboot(true);
        gotoPage(Page::BootselMsg);
        break;
    case Descriptor::Action::GotoParent:
        gotoParent();
        break;
    case Descriptor::Action::None:
        break;
    }
}

void Menu::performValueAction(Menu::Descriptor::Action action, uint8_t value) {
    auto descriptor_it = descriptors.find(m_state_stack.top().page);
    if (descriptor_it == descriptors.end()) {
        assert(false);
        return;
    }

    switch (action) {
    case Descriptor::Action::SetLedBrightness:
        m_store->setLedBrightness(value);
        break;
    case Descriptor::Action::SetLedAnimationSpeed:
        m_store->setLedAnimationSpeed(value);
        break;
    default:
        break;
    }
}

void Menu::performToggleAction(Menu::Descriptor::Action action, bool toggle) {
    auto descriptor_it = descriptors.find(m_state_stack.top().page);
    if (descriptor_it == descriptors.end()) {
        assert(false);
        return;
    }

    switch (action) {
    case Descriptor::Action::SetUsePlayerColor:
        m_store->setUsePlayerColor(toggle);
        break;
    default:
        break;
    }
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
    } else if (pressed.north) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
            if (current_state.selection > 0) {
                current_state.selection--;
                performValueAction(descriptor_it->second.items.at(0).second, current_state.selection);
            }
            break;
        case Descriptor::Type::Toggle:
            current_state.selection = !current_state.selection;
            performToggleAction(descriptor_it->second.items.at(0).second, current_state.selection);
            break;
        case Descriptor::Type::Selection:
        case Descriptor::Type::Root:
            if (current_state.selection == 0) {
                current_state.selection = descriptor_it->second.items.size() - 1;
            } else {
                current_state.selection--;
            }
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.west) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
            if (current_state.selection < UINT8_MAX) {
                current_state.selection++;
                performValueAction(descriptor_it->second.items.at(0).second, current_state.selection);
            }
            break;
        case Descriptor::Type::Toggle:
            current_state.selection = !current_state.selection;
            performToggleAction(descriptor_it->second.items.at(0).second, current_state.selection);
            break;
        case Descriptor::Type::Selection:
        case Descriptor::Type::Root:
            if (current_state.selection == descriptor_it->second.items.size() - 1) {
                current_state.selection = 0;
            } else {
                current_state.selection++;
            }
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.south) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
        case Descriptor::Type::Toggle:
        case Descriptor::Type::Selection:
            gotoParent();
            break;
        case Descriptor::Type::Root:
            m_active = false;
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.east) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
        case Descriptor::Type::Toggle:
            performSelectionAction(descriptor_it->second.items.at(0).second);
            break;
        case Descriptor::Type::Selection:
        case Descriptor::Type::Root:
            performSelectionAction(descriptor_it->second.items.at(current_state.selection).second);
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    }
}

bool Menu::active() { return m_active; }

Menu::State Menu::getState() { return m_state_stack.top(); }

} // namespace Divacon::Utils
