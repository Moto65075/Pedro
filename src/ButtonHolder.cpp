#include "ButtonHolder.h"

ButtonHolder::ButtonHolder(gpio_num_t pin, uint32_t longPressMs)
: _pin(pin), _longPressMs(longPressMs)
{
    // TTP223 não precisa de Pullup interno, ele já empurra 3.3V ou 0V
    pinMode(_pin, INPUT);
}

void ButtonHolder::update() {
    // No Touch TTP223: HIGH = Pressionado, LOW = Solto
    bool level = digitalRead(_pin); 
    uint32_t now = millis();

    // Detecta mudança de estado (Toque ou Soltura)
    if (level != _lastLevel) {
        _lastLevel = level;

        if (level == HIGH) { // Começou a tocar
            _isPressing  = true;
            _shortFired  = false;
            _longFired   = false;
            _pressedTime = now;
        } else {             // Parou de tocar
            _isPressing = false;
            // Se soltou antes de virar Long Press, ativa o Short Press
            if (!_longFired && (now - _pressedTime) < _longPressMs) {
                _shortFired = true;
            }
        }
    }

    // Lógica de Long Press: dispara se continuar segurando após o tempo definido
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
