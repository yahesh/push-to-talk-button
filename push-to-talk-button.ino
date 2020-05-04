/*
  Push-to-Talk-Button
  (c) Yahe 2020
  
  # Debian Gnome:
  
  1.) Create a custom keyboard shortcut named "MUTE" with command `amixer set Capture nocap` and shortcut SHIFT CTRL SUPER M
  2.) Create a custom keyboard shortcut named "UNMUTE" with command `amixer set Capture cap` and shortcut SHIFT CTRL SUPER U

  # macOS:

  1.) Install iCanHazShortcut (https://github.com/deseven/icanhazshortcut), e.g. via Homebrew `brew cask install icanhazshortcut`
  2.) Start iCanHazShortcut and go the Shortcuts tab of the Preferences window
  3.) Create a shortcut named "MUTE" with command `osascript -e "set volume input volume 0"` and shortcut SHIFT CTRL CMD M
  4.) Create a shortcut named "UNMUTE" with command `osascript -e "set volume input volume 75"` and shortcut SHIFT CTRL CMD U
*/

#define BUTTON_PIN              2
#define DEBOUNCE_DELAY         10
#define CLICK_DURATION        250
#define DOUBLE_CLICK_DISTANCE 500

#define COMMAND_NONE      0
#define COMMAND_MUTE      1
#define COMMAND_UNMUTE    2
#define COMMAND_ALTERNATE 3

#define LOOP_STATE_EXIT                      0
#define LOOP_STATE_MUTE                      1
#define LOOP_STATE_UNMUTE                    2
#define LOOP_STATE_ALTERNATE                 3
#define LOOP_STATE_INIT_DEBOUNCE             4
#define LOOP_STATE_WAIT_CLICK_RELEASE        5
#define LOOP_STATE_WAIT_DOUBLE_CLICK_PRESS   6
#define LOOP_STATE_WAIT_DOUBLE_CLICK_RELEASE 7

int16_t BUTTON_STATE = HIGH;
int16_t LAST_COMMAND = COMMAND_NONE;

uint32_t get_time_distance(uint32_t current_time, uint32_t last_time) {
  return (current_time - last_time);
}

void send_command(uint8_t command) {
  switch (command) {
    case COMMAND_MUTE: {
      Keyboard.releaseAll();
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(KEY_LEFT_SHIFT);
      Keyboard.press(KEY_LEFT_GUI);
      Keyboard.press('m');
      Keyboard.releaseAll();
      
      // store the last command for the next run
      LAST_COMMAND = command;

      break;
    }

    case COMMAND_UNMUTE: {
      Keyboard.releaseAll();
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(KEY_LEFT_SHIFT);
      Keyboard.press(KEY_LEFT_GUI);
      Keyboard.press('u');
      Keyboard.releaseAll();
      
      // store the last command for the next run
      LAST_COMMAND = command;

      break;
    }

    case COMMAND_ALTERNATE: {
      switch (LAST_COMMAND) {
        case COMMAND_MUTE: {
          send_command(COMMAND_UNMUTE);
          break;
        }

        case COMMAND_UNMUTE: {
          send_command(COMMAND_MUTE);
          break;
        }
      }
    }
  }
}

void setup() {
  // read the button via pull-up resistor
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // initialize the keyboard
  Keyboard.begin();
}

void loop() {
  // read the button state 
  int16_t button_state  = digitalRead(BUTTON_PIN);
  int16_t button_state2 = button_state;

  uint32_t current_time = 0;

  uint8_t loop_state = LOOP_STATE_EXIT;  
  
  // if the button state changed then debounce the button
  if (BUTTON_STATE != button_state) {
    // start the state machine
    loop_state = LOOP_STATE_INIT_DEBOUNCE;
    do {
      switch (loop_state) {
        case LOOP_STATE_MUTE: {
          send_command(COMMAND_MUTE);
          loop_state = LOOP_STATE_EXIT;
          break;
        }

        case LOOP_STATE_UNMUTE: {
          send_command(COMMAND_UNMUTE);
          loop_state = LOOP_STATE_EXIT;
          break;
        }

        case LOOP_STATE_ALTERNATE: {
          send_command(COMMAND_ALTERNATE);
          loop_state = LOOP_STATE_EXIT;
          break;
        }

        case LOOP_STATE_INIT_DEBOUNCE:{
          // store the current time
          current_time  = millis();

          // if there is no button status change we will exit
          loop_state = LOOP_STATE_EXIT;
          
          // wait some time
          delay(DEBOUNCE_DELAY);

          // read the button state again
          button_state  = digitalRead(BUTTON_PIN);
          button_state2 = button_state;

          if (BUTTON_STATE != button_state) { // the state has still changed
            if (LOW == button_state) { // the button has been pressed
              loop_state = LOOP_STATE_WAIT_CLICK_RELEASE;
            } else { // the button has been released
              loop_state = LOOP_STATE_MUTE;
            }
          }

          break;
        }

        case LOOP_STATE_WAIT_CLICK_RELEASE: {
          // store the current time
          current_time  = millis();

          // if there is no click release we will unmute
          loop_state = LOOP_STATE_UNMUTE;
          
          // we wait for the click release
          do {
            // wait some time
            delay(DEBOUNCE_DELAY);

            // read the button state again
            button_state = digitalRead(BUTTON_PIN);
          
            // if the button state changed then debounce the button
            if (button_state != button_state2) {
              // wait some time
              delay(DEBOUNCE_DELAY);

              // read the button state again
              button_state = digitalRead(BUTTON_PIN);

              // proceed if the state has still changed
              if (button_state != button_state2) {
                // there was a click release so we wait for the double click press
                loop_state = LOOP_STATE_WAIT_DOUBLE_CLICK_PRESS;

                // reset the comparator
                button_state2 = button_state;

                // exit this loop
                break;
              }
            }
          } while (get_time_distance(millis(), current_time) < CLICK_DURATION);

          break;
        }

        case LOOP_STATE_WAIT_DOUBLE_CLICK_PRESS: {
          // store the current time
          current_time  = millis();

          // if there is no double click press we will exit
          loop_state = LOOP_STATE_EXIT;

          // we wait for the double click press
          do {
            // wait some time
            delay(DEBOUNCE_DELAY);

            // read the button state again
            button_state = digitalRead(BUTTON_PIN);
          
            // if the button state changed then debounce the button
            if (button_state != button_state2) {
              // wait some time
              delay(DEBOUNCE_DELAY);

              // read the button state again
              button_state = digitalRead(BUTTON_PIN);

              // proceed if the state has still changed
              if (button_state != button_state2) {
                // there was a double click press so we waut for the double click release
                loop_state = LOOP_STATE_WAIT_DOUBLE_CLICK_RELEASE;

                // reset the comparator
                button_state2 = button_state;

                // exit this loop
                break;
              }
            }
          } while (get_time_distance(millis(), current_time) < DOUBLE_CLICK_DISTANCE);

          break;
        }

        case LOOP_STATE_WAIT_DOUBLE_CLICK_RELEASE: {
          // store the current time
          current_time  = millis();

          // if there is no double click release we will unmute
          loop_state = LOOP_STATE_UNMUTE;
          
          // we wait for the double click release
          do {
            // wait some time
            delay(DEBOUNCE_DELAY);

            // read the button state again
            button_state = digitalRead(BUTTON_PIN);
          
            // if the button state changed then debounce the button
            if (button_state != button_state2) {
              // wait some time
              delay(DEBOUNCE_DELAY);

              // read the button state again
              button_state = digitalRead(BUTTON_PIN);

              // proceed if the state has still changed
              if (button_state != button_state2) {
                // there was a double click release so we alternate the microphone
                loop_state = LOOP_STATE_ALTERNATE;

                // reset the comparator
                button_state2 = button_state;

                // exit this loop
                break;
              }
            }
          } while (get_time_distance(millis(), current_time) < CLICK_DURATION);

          break;
        }
      }
    } while (LOOP_STATE_EXIT != loop_state);

    // store the button state for the next run
    BUTTON_STATE = button_state;
  }

  // wait a bit before the next run
  delay(10);
}
