/**
 * @file buttons.cpp
 * @brief Buttons source file.
 *
 * @authors Gustavo Pivoto Ambrósio
 * @date 2026-06-14
 */

/* Includes ----------------------------------------------------------------- */
#include "buttons.h"

/* Private defines ---------------------------------------------------------- */

/* Private macros ----------------------------------------------------------- */

/* Private typedefs --------------------------------------------------------- */

/* Private constants -------------------------------------------------------- */

/* Private variables -------------------------------------------------------- */

/* Private function prototypes ---------------------------------------------- */

/* Public functions --------------------------------------------------------- */
void buttons_init() {
  pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
}

bool button_leftPressed() {
  return digitalRead(BUTTON_LEFT_PIN) == LOW;
}

bool button_rightPressed() {
  return digitalRead(BUTTON_RIGHT_PIN) == LOW;
}

bool button_selectPressed() {
  return digitalRead(BUTTON_SELECT_PIN) == LOW;
}

/* Private functions -------------------------------------------------------- */
