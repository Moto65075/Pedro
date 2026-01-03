#ifndef BUTTON_HOLDER_H
#define BUTTON_HOLDER_H

#include <Arduino.h>

class ButtonHolder {
public:
    ButtonHolder(gpio_num_t pin, uint32_t longPressMs = 800);

    void update();          // chama em todo loop
    bool shortPressed();    // evento: clique curto (1 vez)
    bool longPressed();     // evento: long press (1 vez, sem precisar soltar)

private:
    gpio_num_t _pin;
    uint32_t _longPressMs;

    bool _lastLevel   = HIGH;
    bool _isPressing  = false;
    bool _shortFired  = false;
    bool _longFired   = false;
    uint32_t _pressedTime = 0;
    uint32_t _lastChange  = 0;
};

#endif