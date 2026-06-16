#ifndef INFRA_BUTTONS_H_
#define INFRA_BUTTONS_H_

#include <Arduino.h>

#define BUTTONS_MOCK_ENABLED 1

#define BUTTON_LEFT_PIN 21
#define BUTTON_RIGHT_PIN 25
#define BUTTON_SELECT_PIN 26
#define BUTTON_BACK_PIN 27

void buttons_init();
void buttons_update();

bool button_leftPressed();
bool button_rightPressed();
bool button_selectPressed();
bool button_backPressed();

bool button_leftClicked();
bool button_rightClicked();
bool button_selectClicked();
bool button_backClicked();

#endif  // INFRA_BUTTONS_H_
