/**
 * @file buttons.h
 * @brief Buttons header file.
 *
 * @author Gustavo Pivoto Ambrósio
 * @date 2026-06-13
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

/* Includes ----------------------------------------------------------------- */
#include <Arduino.h>

/* Defines ------------------------------------------------------------------ */
#define BUTTON_LEFT_PIN 0
#define BUTTON_RIGHT_PIN 4
#define BUTTON_SELECT_PIN 12

/* Macros ------------------------------------------------------------------- */

/* Typedefs ----------------------------------------------------------------- */

/* Constants ---------------------------------------------------------------- */

/* Public function prototypes ----------------------------------------------- */
void buttons_init();

bool button_leftPressed();

bool button_rightPressed();

bool button_selectPressed();

#endif /* BUTTONS_H_ */
