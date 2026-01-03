#include "ButtonHolder.h"

ButtonHolder::ButtonHolder(gpio_num_t pin, uint32_t longPressMs)
: _pin(pin), _longPressMs(longPressMs)
{
    pinMode(_pin, INPUT_PULLUP);
}

void ButtonHolder::update() {
    bool level = digitalRead(_pin);      // HIGH solto, LOW pressionado
    uint32_t now = millis();

    // debounce
    if (level != _lastLevel && (now - _lastChange) > 20) {
        _lastChange = now;
        _lastLevel  = level;

        if (level == LOW) {              // começou a pressionar
            _isPressing  = true;
            _shortFired  = false;
            _longFired   = false;
            _pressedTime = now;
        } else {                         // soltou
            _isPressing = false;
            // se não virou long, libera short
            if (!_longFired && (now - _pressedTime) < _longPressMs) {
                _shortFired = true;
            }
        }
    }

    // long press enquanto segura
    if (_isPressing && !_longFired && (now - _pressedTime) >= _longPressMs) {
        _longFired = true;
    }
}

bool ButtonHolder::shortPressed() {
    if (_shortFired) {
        _shortFired = false;
        return true;
    }
    return false;
}

bool ButtonHolder::longPressed() {
    if (_longFired) {
        _longFired = false;
        return true;
    }
    return false;
}