#include "infra/buttons.h"

static bool previousLeft = false;
static bool previousRight = false;
static bool previousSelect = false;
static bool previousBack = false;
static bool clickedLeft = false;
static bool clickedRight = false;
static bool clickedSelect = false;
static bool clickedBack = false;

void buttons_init() {
#if BUTTONS_MOCK_ENABLED
  Serial.println(
      "Botoes simulados: a=esquerda, d=direita, s/ENTER=selecionar, w=voltar");
#else
  pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_BACK_PIN, INPUT_PULLUP);
#endif
}

void buttons_update() {
#if BUTTONS_MOCK_ENABLED
  clickedLeft = false;
  clickedRight = false;
  clickedSelect = false;
  clickedBack = false;

  while (Serial.available() > 0) {
    char comando = Serial.read();
    if (comando == 'a' || comando == 'A') {
      clickedLeft = true;
    } else if (comando == 'd' || comando == 'D') {
      clickedRight = true;
    } else if (comando == 's' || comando == 'S' || comando == '\r' ||
               comando == '\n') {
      clickedSelect = true;
    } else if (comando == 'w' || comando == 'W') {
      clickedBack = true;
    }
  }
#else
  bool currentLeft = button_leftPressed();
  bool currentRight = button_rightPressed();
  bool currentSelect = button_selectPressed();
  bool currentBack = button_backPressed();

  clickedLeft = currentLeft && !previousLeft;
  clickedRight = currentRight && !previousRight;
  clickedSelect = currentSelect && !previousSelect;
  clickedBack = currentBack && !previousBack;

  previousLeft = currentLeft;
  previousRight = currentRight;
  previousSelect = currentSelect;
  previousBack = currentBack;
#endif
}

bool button_leftPressed() {
#if BUTTONS_MOCK_ENABLED
  return clickedLeft;
#else
  return digitalRead(BUTTON_LEFT_PIN) == LOW;
#endif
}

bool button_rightPressed() {
#if BUTTONS_MOCK_ENABLED
  return clickedRight;
#else
  return digitalRead(BUTTON_RIGHT_PIN) == LOW;
#endif
}

bool button_selectPressed() {
#if BUTTONS_MOCK_ENABLED
  return clickedSelect;
#else
  return digitalRead(BUTTON_SELECT_PIN) == LOW;
#endif
}

bool button_backPressed() {
#if BUTTONS_MOCK_ENABLED
  return clickedBack;
#else
  return digitalRead(BUTTON_BACK_PIN) == LOW;
#endif
}

bool button_leftClicked() { return clickedLeft; }
bool button_rightClicked() { return clickedRight; }
bool button_selectClicked() { return clickedSelect; }
bool button_backClicked() { return clickedBack; }
