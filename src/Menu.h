#ifndef MENU_H
#define MENU_H

// #include <Arduino.h>
#include "EnergyManager.h"
#include "Web.h"
/*
#include <WakeOnLan.h>
#include <WiFiUDP.h>*/

// Estados de UI principais
enum class UIState {
    IDLE_MODE,      // Olhos livres
    MAIN_MENU,      // Menu principal
    POWER_MENU      // Menu de energia
};

class Menu {
public:
    Menu();

    // Deve ser chamado a cada loop para tratar lógica de botões
    void hold(bool powerShort, bool powerLong,
              bool nextShort);

    // Estado atual da UI (pra você tratar no loop)
    UIState state() const;

    // Dados de navegação atuais
    int  currentIndex() const;
    bool sectionOpened() const;

    // Ações explícitas (se quiser chamar manualmente)
    void mainMenu();
    void powerMenu();
    void select();
    void nextSection();
    void returnTo(UIState ui);

private:
    UIState _currentUI;
    int     _currentSectionIndex;
    bool    _sectionOpened;

    // quantos itens por menu (MAIN / POWER)
    static constexpr int _sectionsMain  = 3; // 0..3
    static constexpr int _sectionsPower = 3; // ajustar depois
};

#endif