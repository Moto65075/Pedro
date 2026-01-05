#ifndef BUTTON_HOLDER_H
#define BUTTON_HOLDER_H

#include <Arduino.h>

class ButtonHolder {
public:
    // Mudamos para INPUT apenas, pois o TTP223 envia sinal ativo
    ButtonHolder(gpio_num_t pin, uint32_t longPressMs = 800);

    void update();          
    bool shortPressed();    
    bool longPressed();     

private:
    gpio_num_t _pin;
    uint32_t _longPressMs;

    // Lógica para Touch: LOW solto, HIGH pressionado
    bool _lastLevel   = LOW; 
    bool _isPressing  = false;
    bool _shortFired  = false;
    bool _longFired   = false;
    uint32_t _pressedTime = 0;
};

#endif
