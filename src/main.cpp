#include "peripherals/ButtonLeds.h"
#include "peripherals/Buttons.h"
#include "peripherals/Display.h"
#include "peripherals/TouchSlider.h"
#include "peripherals/TouchSliderLeds.h"
#include "usb/device/hid/ps4_auth.h"
#include "usb/device_driver.h"
#include "utils/InputReport.h"
#include "utils/InputState.h"
#include "utils/Menu.h"
#include "utils/PS4AuthProvider.h"
#include "utils/SettingsStore.h"

#include "GlobalConfiguration.h"
#include "PS4AuthConfiguration.h"

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include <cstdio>
#include <memory>

using namespace Divacon;

namespace {

queue_t control_queue;
queue_t menu_display_queue;
queue_t input_queue;
queue_t led_queue;

queue_t auth_challenge_queue;
queue_t auth_signed_challenge_queue;

enum class ControlCommand : uint8_t {
    SetUsbMode,
    SetPlayerLed,
    SetButtonLed,
    SetLedBrightness,
    SetLedAnimationSpeed,
    SetLedIdleMode,
    SetLedTouchedMode,
    SetLedIdleColor,
    SetLedTouchedColor,
    SetLedEnablePlayerColor,
    SetLedEnablePdloaderSupport,
    EnterMenu,
    ExitMenu,
};

struct ControlMessage {
    ControlCommand command;
    union {
        usb_mode_t usb_mode;
        usb_player_led_t player_led;
        usb_button_led_t button_led;
        uint8_t led_brightness;
        uint8_t led_animation_speed;
        Peripherals::TouchSliderLeds::Config::IdleMode led_idle_mode;
        Peripherals::TouchSliderLeds::Config::TouchedMode led_touched_mode;
        Peripherals::TouchSliderLeds::Config::Color led_idle_color;
        Peripherals::TouchSliderLeds::Config::Color led_touched_color;
        bool led_enable_player_color;
        bool led_enable_pdloader_support;
    } data;
};

void core1_task() {
    multicore_lockout_victim_init();

    Peripherals::Display display(Config::Default::display_config);
    Peripherals::TouchSliderLeds sliderleds(Config::Default::touch_slider_leds_config);
    Peripherals::ButtonLeds buttonleds(Config::Default::button_leds_config,
                                       Config::Default::touch_slider_leds_config.enable_pdloader_support);

    Utils::PS4AuthProvider ps4authprovider;
    std::array<uint8_t, Utils::PS4AuthProvider::SIGNATURE_LENGTH> auth_challenge{};

    Utils::InputState::InputMessage input_msg{};
    Utils::Menu::State menu_display_msg{};
    Peripherals::TouchSliderLeds::RawFrameMessage slider_led_msg;
    ControlMessage control_msg{};

    while (true) {
        if (queue_try_remove(&control_queue, &control_msg)) {
            switch (control_msg.command) {
            case ControlCommand::SetUsbMode:
                display.setUsbMode(control_msg.data.usb_mode);
                break;
            case ControlCommand::SetPlayerLed:
                switch (control_msg.data.player_led.type) {
                case USB_PLAYER_LED_ID:
                    display.setPlayerId(control_msg.data.player_led.id);
                    break;
                case USB_PLAYER_LED_COLOR:
                    sliderleds.setPlayerColor({.r = control_msg.data.player_led.red,
                                               .g = control_msg.data.player_led.green,
                                               .b = control_msg.data.player_led.blue});
                }
                break;
            case ControlCommand::SetButtonLed:
                buttonleds.update(control_msg.data.button_led);
                break;
            case ControlCommand::SetLedBrightness:
                sliderleds.setBrightness(control_msg.data.led_brightness);
                break;
            case ControlCommand::SetLedAnimationSpeed:
                sliderleds.setAnimationSpeed(control_msg.data.led_animation_speed);
                break;
            case ControlCommand::SetLedIdleMode:
                sliderleds.setIdleMode(control_msg.data.led_idle_mode);
                break;
            case ControlCommand::SetLedTouchedMode:
                sliderleds.setTouchedMode(control_msg.data.led_touched_mode);
                break;
            case ControlCommand::SetLedIdleColor:
                sliderleds.setIdleColor(control_msg.data.led_idle_color);
                break;
            case ControlCommand::SetLedTouchedColor:
                sliderleds.setTouchedColor(control_msg.data.led_touched_color);
                break;
            case ControlCommand::SetLedEnablePlayerColor:
                sliderleds.setEnablePlayerColor(control_msg.data.led_enable_player_color);
                break;
            case ControlCommand::SetLedEnablePdloaderSupport:
                sliderleds.setEnablePdloaderSupport(control_msg.data.led_enable_pdloader_support);
                buttonleds.setEnablePdloaderSupport(control_msg.data.led_enable_pdloader_support);
                break;
            case ControlCommand::EnterMenu:
                display.showMenu();
                break;
            case ControlCommand::ExitMenu:
                display.showIdle();
                break;
            }
        }
        if (queue_try_remove(&input_queue, &input_msg)) {
            sliderleds.setTouched(input_msg.touches);
            buttonleds.setButtons(input_msg.buttons);
            display.setTouched(input_msg.touches);
            display.setButtons(input_msg.buttons);
        }
        if (queue_try_remove(&led_queue, &slider_led_msg)) {
            sliderleds.update(slider_led_msg);
        } else {
            sliderleds.update();
        }
        if (queue_try_remove(&menu_display_queue, &menu_display_msg)) {
            display.setMenuState(menu_display_msg);
        }
        if (queue_try_remove(&auth_challenge_queue, auth_challenge.data())) {
            display.setAuthBusyState(true);
            display.update(true);

            const auto signed_challenge = ps4authprovider.sign(auth_challenge);
            queue_try_remove(&auth_signed_challenge_queue, nullptr); // clear queue first
            queue_try_add(&auth_signed_challenge_queue, &signed_challenge);

            display.setAuthBusyState(false);
        }

        buttonleds.update();
        display.update();
    }
}

} // namespace

int main() {
    queue_init(&control_queue, sizeof(ControlMessage), 1);
    queue_init(&menu_display_queue, sizeof(Utils::Menu::State), 1);
    queue_init(&input_queue, sizeof(Utils::InputState::InputMessage), 1);
    queue_init(&led_queue, sizeof(Peripherals::TouchSliderLeds::RawFrameMessage), 1);
    queue_init(&auth_challenge_queue, sizeof(std::array<uint8_t, Utils::PS4AuthProvider::SIGNATURE_LENGTH>), 1);
    queue_init(&auth_signed_challenge_queue, sizeof(std::array<uint8_t, Utils::PS4AuthProvider::SIGNATURE_LENGTH>), 1);

    Peripherals::Buttons buttons(Config::Default::buttons_config);

    Utils::InputReport input_report;
    Utils::InputState input_state;
    const auto checkHotkey = [&input_state]() {
        static const uint32_t hold_timeout = 2000;
        static uint32_t hold_since = 0;
        static bool hold_active = false;

        if (input_state.buttons.start && input_state.buttons.select) {
            const uint32_t now = to_ms_since_boot(get_absolute_time());
            if (!hold_active) {
                hold_active = true;
                hold_since = now;
            } else if ((now - hold_since) > hold_timeout) {
                hold_active = false;
                return true;
            }
        } else {
            hold_active = false;
        }
        return false;
    };

    auto settings_store = std::make_shared<Utils::SettingsStore>();
    const auto mode = settings_store->getUsbMode();
    const auto readSettings = [&]() {
        const auto sendCtrlMessage = [&](const ControlMessage &msg) { queue_add_blocking(&control_queue, &msg); };

        sendCtrlMessage({.command = ControlCommand::SetUsbMode, .data = {.usb_mode = mode}});
        sendCtrlMessage({.command = ControlCommand::SetLedBrightness,
                         .data = {.led_brightness = settings_store->getLedBrightness()}});
        sendCtrlMessage({.command = ControlCommand::SetLedAnimationSpeed,
                         .data = {.led_animation_speed = settings_store->getLedAnimationSpeed()}});
        sendCtrlMessage(
            {.command = ControlCommand::SetLedIdleMode, .data = {.led_idle_mode = settings_store->getLedIdleMode()}});
        sendCtrlMessage({.command = ControlCommand::SetLedTouchedMode,
                         .data = {.led_touched_mode = settings_store->getLedTouchedMode()}});
        sendCtrlMessage({.command = ControlCommand::SetLedIdleColor,
                         .data = {.led_idle_color = settings_store->getLedIdleColor()}});
        sendCtrlMessage({.command = ControlCommand::SetLedTouchedColor,
                         .data = {.led_touched_color = settings_store->getLedTouchedColor()}});
        sendCtrlMessage({.command = ControlCommand::SetLedEnablePlayerColor,
                         .data = {.led_enable_player_color = settings_store->getLedEnablePlayerColor()}});
        sendCtrlMessage({.command = ControlCommand::SetLedEnablePdloaderSupport,
                         .data = {.led_enable_pdloader_support = settings_store->getLedEnablePdloaderSupport()}});

        buttons.setMirrorToDpad(settings_store->getInputMirrorToDpad());
    };

    Peripherals::TouchSlider touch_slider(Config::Default::touch_slider_config, mode);
    Utils::Menu menu(settings_store);

    std::array<uint8_t, Utils::PS4AuthProvider::SIGNATURE_LENGTH> auth_challenge_response{};
    if (Config::PS4Auth::config.enabled) {
        ps4_auth_init(Config::PS4Auth::config.key_pem.c_str(), Config::PS4Auth::config.key_pem.size() + 1,
                      Config::PS4Auth::config.serial.data(), Config::PS4Auth::config.signature.data(),
                      [](const uint8_t *challenge) { queue_try_add(&auth_challenge_queue, challenge); });
    }

    multicore_launch_core1(core1_task);

    usbd_driver_init(mode);
    stdio_init_all();

    usbd_driver_set_player_led_cb([](usb_player_led_t player_led) {
        const auto ctrl_message =
            ControlMessage{.command = ControlCommand::SetPlayerLed, .data = {.player_led = player_led}};
        queue_try_add(&control_queue, &ctrl_message);
    });
    usbd_driver_set_button_led_cb([](usb_button_led_t button_led) {
        const auto ctrl_message =
            ControlMessage{.command = ControlCommand::SetButtonLed, .data = {.button_led = button_led}};
        queue_try_add(&control_queue, &ctrl_message);
    });
    usbd_driver_set_slider_led_cb([](const uint8_t *frame, size_t len) {
        auto led_message = Peripherals::TouchSliderLeds::RawFrameMessage();

        // Colors in `frame` 3 byte in the order: Green, Red, Blue from right
        // to left (so we need to reverse the order).
        for (size_t color_idx = 0; color_idx < len / 3; ++color_idx) {
            if (color_idx >= led_message.size()) {
                break;
            }

            led_message.at(led_message.size() - 1 - color_idx) =
                Peripherals::TouchSliderLeds::TouchSliderLeds::Config::Color{
                    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    .r = frame[(color_idx * 3) + 1],
                    .g = frame[(color_idx * 3)],
                    .b = frame[(color_idx * 3) + 2],
                    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                };
        }

        queue_try_add(&led_queue, &led_message);
    });

    readSettings();

    while (true) {
        buttons.updateInputState(input_state);
        touch_slider.updateInputState(input_state);

        const auto input_message = input_state.getInputMessage();

        if (menu.active()) {
            menu.update(input_state);
            if (menu.active()) {
                const auto display_msg = menu.getState();
                queue_add_blocking(&menu_display_queue, &display_msg);
            } else {
                settings_store->store();

                ControlMessage ctrl_message = {.command = ControlCommand::ExitMenu, .data = {}};
                queue_add_blocking(&control_queue, &ctrl_message);
            }

            readSettings();
            input_state.releaseAll();

        } else if (checkHotkey()) {
            menu.activate();

            ControlMessage ctrl_message{.command = ControlCommand::EnterMenu, .data = {}};
            queue_add_blocking(&control_queue, &ctrl_message);
        }

        usbd_driver_send_report(input_report.getReport(input_state, mode));
        usbd_driver_task();

        queue_try_add(&input_queue, &input_message);

        if (queue_try_remove(&auth_signed_challenge_queue, auth_challenge_response.data())) {
            ps4_auth_set_signed_challenge(auth_challenge_response.data());
        }
    }

    return 0;
}