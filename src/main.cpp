#include "peripherals/Buttons.h"
#include "peripherals/Display.h"
#include "peripherals/TouchSlider.h"
#include "peripherals/TouchSliderLeds.h"
#include "usb/device/device_driver.h"
#include "utils/InputState.h"
#include "utils/Menu.h"
#include "utils/SettingsStore.h"

#include "GlobalConfiguration.h"

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include <memory>
#include <stdio.h>

using namespace Divacon;

queue_t control_queue;
queue_t input_queue;
queue_t menu_display_queue;

enum class ControlCommand {
    SetUsbMode,
    SetPlayerLed,
    SetLedBrightness,
    EnterMenu,
    ExitMenu,
};

struct ControlMessage {
    ControlCommand command;
    union {
        usb_mode_t usb_mode;
        usb_player_led_t player_led;
        uint8_t brightness;
    } data;
};

void core1_task() {
    multicore_lockout_victim_init();

    Peripherals::Display display(Config::Default::display_config);
    Peripherals::TouchSliderLeds sliderled(Config::Default::touch_slider_leds_config);

    ControlMessage control_msg;
    Utils::InputState::InputMessage input_msg;
    Utils::Menu::State menu_display_msg;

    while (true) {
        if (queue_try_remove(&control_queue, &control_msg)) {
            switch (control_msg.command) {
            case ControlCommand::SetUsbMode:
                display.setUsbMode(control_msg.data.usb_mode);
                break;
            case ControlCommand::SetPlayerLed:
                if (control_msg.data.player_led.type == USB_PLAYER_LED_ID) {
                    display.setPlayerId(control_msg.data.player_led.id);
                } else if (control_msg.data.player_led.type == USB_PLAYER_LED_COLOR) {
                    sliderled.setPlayerColor({control_msg.data.player_led.red, control_msg.data.player_led.green,
                                              control_msg.data.player_led.blue});
                }
                break;
            case ControlCommand::SetLedBrightness:
                sliderled.setBrightness(control_msg.data.brightness);
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
            sliderled.setTouched(input_msg.touches);
            display.setTouched(input_msg.touches);
            display.setButtons(input_msg.buttons);
        }
        if (queue_try_remove(&menu_display_queue, &menu_display_msg)) {
            display.setMenuState(menu_display_msg);
        }

        sliderled.update();
        display.update();

        sleep_ms(1);
    }
}

int main() {
    queue_init(&control_queue, sizeof(ControlMessage), 1);
    queue_init(&input_queue, sizeof(Utils::InputState::InputMessage), 1);
    queue_init(&menu_display_queue, sizeof(Utils::Menu::State), 1);

    Utils::InputState input_state;

    auto settings_store = std::make_shared<Utils::SettingsStore>();
    Utils::Menu menu(settings_store);

    Peripherals::TouchSlider touch_slider(Config::Default::touch_slider_config);
    Peripherals::Buttons buttons(Config::Default::buttons_config);

    multicore_launch_core1(core1_task);

    auto mode = settings_store->getUsbMode();
    usb_device_driver_init(mode);
    usb_device_driver_set_player_led_cb([](usb_player_led_t player_led) {
        auto ctrl_message = ControlMessage{ControlCommand::SetPlayerLed, {.player_led = player_led}};
        queue_add_blocking(&control_queue, &ctrl_message);
    });

    stdio_init_all();

    auto readSettings = [&]() {
        ControlMessage ctrl_message;

        ctrl_message = {ControlCommand::SetUsbMode, {.usb_mode = mode}};
        queue_add_blocking(&control_queue, &ctrl_message);

        ctrl_message = {ControlCommand::SetLedBrightness, {.brightness = settings_store->getLedBrightness()}};
        queue_add_blocking(&control_queue, &ctrl_message);

        touch_slider.setMode(settings_store->getSliderMode());
    };

    readSettings();

    while (true) {
        buttons.updateInputState(input_state);
        touch_slider.updateInputState(input_state);

        if (menu.active()) {
            menu.update(input_state);
            if (menu.active()) {
                auto display_msg = menu.getState();
                queue_add_blocking(&menu_display_queue, &display_msg);
            } else {
                settings_store->store();

                ControlMessage ctrl_message = {ControlCommand::ExitMenu, {}};
                queue_add_blocking(&control_queue, &ctrl_message);
            }

            readSettings();

        } else if (input_state.checkHotkey()) {
            menu.activate();

            input_state.releaseAll();
            usb_device_driver_send_and_receive_report(input_state.getReport(mode));

            ControlMessage ctrl_message{ControlCommand::EnterMenu, {}};
            queue_add_blocking(&control_queue, &ctrl_message);
        } else {
            usb_device_driver_send_and_receive_report(input_state.getReport(mode));
        }

        usb_device_driver_task();

        auto input_message = input_state.getInputMessage();
        queue_try_add(&input_queue, &input_message);
    }

    return 0;
}