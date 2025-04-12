#include "peripherals/ButtonLeds.h"
#include "peripherals/Buttons.h"
#include "peripherals/Display.h"
#include "peripherals/TouchSlider.h"
#include "peripherals/TouchSliderLeds.h"
#include "usb/device/hid/ps4_auth.h"
#include "usb/device_driver.h"
#include "utils/InputState.h"
#include "utils/Menu.h"
#include "utils/PS4AuthProvider.h"
#include "utils/SettingsStore.h"

#include "GlobalConfiguration.h"
#include "PS4AuthConfiguration.h"

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include <memory>
#include <stdio.h>

using namespace Divacon;

queue_t control_queue;
queue_t menu_display_queue;
queue_t input_queue;
queue_t led_queue;

queue_t auth_challenge_queue;
queue_t auth_signed_challenge_queue;

enum class ControlCommand {
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
    std::array<uint8_t, Utils::PS4AuthProvider::SIGNATURE_LENGTH> auth_challenge;

    ControlMessage control_msg;
    Utils::Menu::State menu_display_msg;
    Utils::InputState::InputMessage input_msg;
    Peripherals::TouchSliderLeds::RawFrameMessage slider_led_msg;

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
                    sliderleds.setPlayerColor({control_msg.data.player_led.red, control_msg.data.player_led.green,
                                               control_msg.data.player_led.blue});
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
            const auto signed_challenge = ps4authprovider.sign(auth_challenge);
            queue_try_remove(&auth_signed_challenge_queue, nullptr); // clear queue first
            queue_try_add(&auth_signed_challenge_queue, &signed_challenge);
        }

        buttonleds.update();
        display.update();

        sleep_ms(1);
    }
}

int main() {
    queue_init(&control_queue, sizeof(ControlMessage), 1);
    queue_init(&menu_display_queue, sizeof(Utils::Menu::State), 1);
    queue_init(&input_queue, sizeof(Utils::InputState::InputMessage), 1);
    queue_init(&led_queue, sizeof(Peripherals::TouchSliderLeds::RawFrameMessage), 1);
    queue_init(&auth_challenge_queue, sizeof(std::array<uint8_t, Utils::PS4AuthProvider::SIGNATURE_LENGTH>), 1);
    queue_init(&auth_signed_challenge_queue, sizeof(std::array<uint8_t, Utils::PS4AuthProvider::SIGNATURE_LENGTH>), 1);

    Utils::InputState input_state;
    std::array<uint8_t, Utils::PS4AuthProvider::SIGNATURE_LENGTH> auth_challenge_response;

    auto settings_store = std::make_shared<Utils::SettingsStore>();
    Utils::Menu menu(settings_store);

    const auto mode = settings_store->getUsbMode();

    Peripherals::TouchSlider touch_slider(Config::Default::touch_slider_config, mode);
    Peripherals::Buttons buttons(Config::Default::buttons_config);

    multicore_launch_core1(core1_task);

    usbd_driver_init(mode);
    usbd_driver_set_player_led_cb([](usb_player_led_t player_led) {
        const auto ctrl_message = ControlMessage{ControlCommand::SetPlayerLed, {.player_led = player_led}};
        queue_try_add(&control_queue, &ctrl_message);
    });
    usbd_driver_set_button_led_cb([](usb_button_led_t button_led) {
        const auto ctrl_message = ControlMessage{ControlCommand::SetButtonLed, {.button_led = button_led}};
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

            led_message[led_message.size() - 1 - color_idx] =
                Peripherals::TouchSliderLeds::TouchSliderLeds::Config::Color{
                    .r = frame[color_idx * 3 + 1],
                    .g = frame[color_idx * 3],
                    .b = frame[color_idx * 3 + 2],
                };
        }

        queue_try_add(&led_queue, &led_message);
    });

    if (Config::PS4Auth::config.enabled) {
        ps4_auth_init(Config::PS4Auth::config.key_pem.c_str(), Config::PS4Auth::config.key_pem.size() + 1,
                      Config::PS4Auth::config.serial.data(), Config::PS4Auth::config.signature.data(),
                      [](const uint8_t *challenge) { queue_try_add(&auth_challenge_queue, challenge); });
    }

    stdio_init_all();

    const auto readSettings = [&]() {
        buttons.setMirrorToDpad(settings_store->getInputMirrorToDpad());

        ControlMessage ctrl_message;

        ctrl_message = {ControlCommand::SetUsbMode, {.usb_mode = mode}};
        queue_add_blocking(&control_queue, &ctrl_message);

        ctrl_message = {ControlCommand::SetLedBrightness, {.led_brightness = settings_store->getLedBrightness()}};
        queue_add_blocking(&control_queue, &ctrl_message);
        ctrl_message = {ControlCommand::SetLedAnimationSpeed,
                        {.led_animation_speed = settings_store->getLedAnimationSpeed()}};
        queue_add_blocking(&control_queue, &ctrl_message);
        ctrl_message = {ControlCommand::SetLedIdleMode, {.led_idle_mode = settings_store->getLedIdleMode()}};
        queue_add_blocking(&control_queue, &ctrl_message);
        ctrl_message = {ControlCommand::SetLedTouchedMode, {.led_touched_mode = settings_store->getLedTouchedMode()}};
        queue_add_blocking(&control_queue, &ctrl_message);
        ctrl_message = {ControlCommand::SetLedIdleColor, {.led_idle_color = settings_store->getLedIdleColor()}};
        queue_add_blocking(&control_queue, &ctrl_message);
        ctrl_message = {ControlCommand::SetLedTouchedColor,
                        {.led_touched_color = settings_store->getLedTouchedColor()}};
        queue_add_blocking(&control_queue, &ctrl_message);
        ctrl_message = {ControlCommand::SetLedEnablePlayerColor,
                        {.led_enable_player_color = settings_store->getLedEnablePlayerColor()}};
        queue_add_blocking(&control_queue, &ctrl_message);
        ctrl_message = {ControlCommand::SetLedEnablePdloaderSupport,
                        {.led_enable_pdloader_support = settings_store->getLedEnablePdloaderSupport()}};
        queue_add_blocking(&control_queue, &ctrl_message);
    };

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

                ControlMessage ctrl_message = {ControlCommand::ExitMenu, {}};
                queue_add_blocking(&control_queue, &ctrl_message);
            }

            readSettings();
            input_state.releaseAll();

        } else if (input_state.checkHotkey()) {
            menu.activate();

            ControlMessage ctrl_message{ControlCommand::EnterMenu, {}};
            queue_add_blocking(&control_queue, &ctrl_message);
        }

        usbd_driver_send_report(input_state.getReport(mode));
        usbd_driver_task();

        queue_try_add(&input_queue, &input_message);

        if (queue_try_remove(&auth_signed_challenge_queue, auth_challenge_response.data())) {
            ps4_auth_set_signed_challenge(auth_challenge_response.data());
        }
    }

    return 0;
}