#include <Joystick.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <MsTimer2.h>

#define BUTTON_SIZE 8
#define BTN_RELEASED 1
#define BTN_PRESSED 0

#define INIT_MOUSE_MODE 5
#define INIT_GAMEPAD_MODE 6

//number of iterations of identical keyscan values before we trigger a keypress
#define DEBOUNCE_ITER 5 
//milliseconds between each scan. SCAN_PERIOD * DEBOUNCE_ITER = minimum response time
#define SCAN_PERIOD 5
//max repetition count reported, should be < max value of the typeof buttonIterCount
#define MAX_REPETITION_COUNT 100

int buttons[] = {2,3,4,5,6,7,8,9};  // map pin numbers to logical key numbers.
unsigned char buttonIterCount[] = {0,0,0,0,0,0,0,0};
//keycodes / keys written in keyboard mode for each key
char keys[] = {KEY_TAB,KEY_UP_ARROW,KEY_RETURN,KEY_ESC,' ',KEY_RIGHT_ARROW,KEY_DOWN_ARROW,KEY_LEFT_ARROW};
//button state, either BTN_PRESSED or BTN_RELEASED for each button.
int buttonState[BUTTON_SIZE];
//current mode, default is keyboard.
enum PadMode {KEYBOARD, MOUSE, GAMEPAD} mode = KEYBOARD;

//LED pins
#define NUM_LEDS 3
int ledPins[] = {21,20,18};

//setup Joystick lib.
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  4, 0,                  // 4 buttons (A,B,X,Y, no hats
  true, true, false,     // two axes, no 'z' axis
  false, false, false,   // RX,RY,RZ
  false, false,          // No rudder or throttle
  false, false, false);  // No accelerator, brake, or steering

void setMode(PadMode inMode) {
  mode = inMode;
  switch(mode) {
    case KEYBOARD:
      digitalWrite(ledPins[0],HIGH);
      break;
    case MOUSE: 
      digitalWrite(ledPins[1],HIGH);
      break;
    case GAMEPAD:
      digitalWrite(ledPins[2],HIGH);
      break;
  }
}

void setup()
{
  //setup pins from 'buttons' array above, set pull up resistors, set the initial button state.
  for (int c1 = 0;c1 < BUTTON_SIZE;c1 ++) {
    pinMode(buttons[c1], INPUT);
    digitalWrite(buttons[c1], HIGH);
    //set initial button state
    buttonState[c1] = digitalRead(buttons[c1]);
  }

  //setup the LED pins
  for (int c1 = 0;c1 < NUM_LEDS;c1 ++) {
    pinMode(ledPins[c1], OUTPUT);
    digitalWrite(ledPins[c1], LOW);
  }
  
  //mode is keyboard by default, check the two init buttons (5,6) to 
  //see if we want to be in mouse or gamepad mode
  if( digitalRead(INIT_MOUSE_MODE) == BTN_PRESSED) {
    setMode(MOUSE);  
  } else if (digitalRead(INIT_GAMEPAD_MODE) == BTN_PRESSED) {
    setMode(GAMEPAD);
  } else {
    setMode(KEYBOARD);
  }
  
  Keyboard.begin();
  Mouse.begin();
  Joystick.begin();
  Joystick.setXAxisRange(-1, 1);
  Joystick.setYAxisRange(-1, 1);
  
  MsTimer2::set(SCAN_PERIOD,keyScan);
  MsTimer2::start();
}

//keyboard handler, IFF transition then send the appropriate keycode.
void keyboardHandler(int key, int state, bool transition, int repetitions) {
  
  if(transition) {
    if(state == BTN_PRESSED) {
      Keyboard.press(keys[key]);
    } else if (state == BTN_RELEASED) {
      Keyboard.release(keys[key]);
    }
  }
}

void mouseHandler(int key, int state, bool transition, int repetitions) {

  if(transition && (key == 0 || key == 2)) {
      char mouseButton = MOUSE_LEFT;
      if(key == 2) mouseButton = MOUSE_RIGHT;
      if(state == BTN_PRESSED) {
        Mouse.press(mouseButton);   
      } else {
        Mouse.release(mouseButton);
      }
  } else if(transition && state == BTN_PRESSED && (key == 3 || key == 4)){
      //the mouse wheel works better as key presses but we just have to capture the pressed
      Mouse.move(0,0,key == 3 ? 1 : -1);
  } else if (state == BTN_PRESSED && (key == 1 || key == 5 || key == 6 || key == 7)) {
    signed char xval = 0, yval = 0;
    int accel = (repetitions / 10)+1;
    if(key == 1) yval = -accel;
    else if(key == 5) xval = accel;
    else if(key == 6) yval = accel;
    else if(key == 7) xval = -accel;
    Mouse.move(xval,yval,0);
  } 
    
}

void gamepadHandler(int key, int state, bool transition, int repetitions) {
  if(transition) { 
    if(key == 0 || key == 2 || key == 3 || key == 4) {
        int keymap[] = {2,-1,3,0,1}; // A and B buttons on RHS, 'X' and 'Y' buttons as top pair
        int padState = state == BTN_PRESSED ? 1 : 0;
        Joystick.setButton(keymap[key], padState);
    } else if (key == 1 || key == 6) { //y axis
        if(state == BTN_RELEASED) Joystick.setYAxis(0);
        else {
          Joystick.setYAxis(key == 1 ? -1 : 1);
        }
    } else if (key == 5 || key == 7) { //x axis
        if(state == BTN_RELEASED) Joystick.setXAxis(0);
        else {
          Joystick.setXAxis(key == 5 ? 1 : -1);  
        }
    }
  }
}

void activeHandler(int key, int state, bool transition, int repetitions) {
    switch(mode) {
      case MOUSE: mouseHandler(key,state, transition, repetitions);
                  break;
      case KEYBOARD: keyboardHandler(key,state,transition, repetitions);
                  break;
      case GAMEPAD:   gamepadHandler(key,state,transition, repetitions);
                  break;
    }
}

void keyScan() {
  for (int c1 = 0;c1 < BUTTON_SIZE;c1 ++) {
     if (digitalRead(buttons[c1]) == BTN_PRESSED && buttonState[c1] == BTN_RELEASED) {
         buttonState[c1] = BTN_PRESSED;
         buttonIterCount[c1] = 0;
         
     } else if (digitalRead(buttons[c1]) == BTN_RELEASED && buttonState[c1] == BTN_PRESSED) {
         buttonState[c1] = BTN_RELEASED;
         buttonIterCount[c1] = 0;
                  
     } else { 
         //debounce logic. If its the same as the existing state and ...
         // 1. debounce is < than the debounce iterations then just increment the count
         // 2. the SAME as debounce iterations, we trigger the transition & increment the count.
         // 3. BIGGER that the debounce iterations, we call the handler but 
         //    with transition set to false, for mouse movement etc
         // we increment the count until we hit MAX_REPETITION_COUNT so there's some 
         // indication of how long the button is held down, for mouse accel etc.
         // TODO: probably better to replace this with elapsed time since pressed.  
         if(buttonIterCount[c1] < DEBOUNCE_ITER) {
            buttonIterCount[c1] ++;
         } else if(buttonIterCount[c1] == DEBOUNCE_ITER) {
            activeHandler(c1, buttonState[c1] ,true,1);
            buttonIterCount[c1]++; //increment once more so we don't do the transition again. 
         } else if (buttonIterCount[c1] > DEBOUNCE_ITER) {
            activeHandler(c1, buttonState[c1] ,false,buttonIterCount[c1] - DEBOUNCE_ITER);  
            if((buttonIterCount[c1] - DEBOUNCE_ITER) < MAX_REPETITION_COUNT) {
              buttonIterCount[c1] ++;
            }
         }        
     }
  }
}

void loop() {
}
