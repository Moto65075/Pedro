#include "Menu.h"

Menu::Menu()
: _currentUI(UIState::IDLE_MODE),
  _currentSectionIndex(0),
  _sectionOpened(false)
{}

UIState Menu::state() const            { return _currentUI; }
int     Menu::currentIndex() const     { return _currentSectionIndex; }
bool    Menu::sectionOpened() const    { return _sectionOpened; }

void Menu::mainMenu() {
    _currentUI = UIState::MAIN_MENU;
    _currentSectionIndex = 0;
    _sectionOpened = false;
}

void Menu::powerMenu() {
    _currentUI = UIState::POWER_MENU;
    _currentSectionIndex = 0;
    _sectionOpened = false;
}

void Menu::returnTo(UIState ui) {
    _currentUI = ui;
    _sectionOpened = false;
    _currentSectionIndex = 0;
}

void Menu::select() {
    if (_sectionOpened) {
        // já está numa tela aberta, POWER curto = voltar ao menu atual
        returnTo(_currentUI);
        return;
    }
     
    if (_currentUI == UIState::MAIN_MENU) {
        switch (_currentSectionIndex) {
            case 0: // Info
                // aqui você desenha FIRMWARE INFO no loop
                break;
            case 1: // Testes
                // desenha tela Tests no loop
                break;
            case 2: // LOGS
                
                break;
            case 3: // Exit
                returnTo(UIState::IDLE_MODE);
                return;
        }
    } else if (_currentUI == UIState::POWER_MENU) {
        // defina os casos de Power (Idle, Night, Light, Off)
        switch (_currentSectionIndex) {
            case 0: // Idle
                returnTo(UIState::IDLE_MODE);
                return;
            case 1: 
                lightSleep();
                returnTo(UIState::IDLE_MODE);
                return;
                break;
            case 2: {
                break;
            }
            case 3:
                
                deepSleep();
                break;
        }
    } 

    // se não saiu, significa que abriu uma “section”
    _sectionOpened = true;
}

void Menu::nextSection() {
    if (_currentUI == UIState::IDLE_MODE) return;

    int maxIndex = (_currentUI == UIState::MAIN_MENU)
                   ? _sectionsMain
                   : _sectionsPower;

    _currentSectionIndex++;
    if (_currentSectionIndex > maxIndex)
        _currentSectionIndex = 0;
}

void Menu::hold(bool powerShort, bool powerLong,
                bool nextShort)
{
    // LONG POWER → POWER_MENU, de qualquer lugar
    if (powerLong) {
        powerMenu();
        return;
    }

    // Botão NEXT (GPIO5) → muda índice, se estiver em menu
    if (nextShort && !_sectionOpened) {
        nextSection();
    }

    // POWER curto → depende do estado
    if (powerShort) {
        switch (_currentUI) {
            case UIState::IDLE_MODE:
                // sai dos olhos e entra no MAIN_MENU
                mainMenu();
                break;

            case UIState::MAIN_MENU:
            case UIState::POWER_MENU:
                // seleciona / volta
                select();
                break;
        }
    }
}