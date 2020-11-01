# multi-micro-macro

Simple sketch for a multi mode Macro pad based off a Micro Pro. 

8 switches, wired directly to Micro pins: 2,3,4,5,6,7,8,9 in the following 4x2 arrangement:

[2][3][4][5]
[9][8][7][6] 

And 3 indicator LEDs denoting which mode the pad is in, on pins 21,20,18

Pad has 3 modes, Keyboard, Mouse, Gamepad. 
Mode is chosen on power up, default is Keyboard. Holding key on the top right (pin 5) on power up chooses Mouse mode, holding key on bottom right (pin 6) on power up chooses gamepad mode.

In Keyboard mode, keys are as follows ..
[KEY_TAB]       [KEY_UP_ARROW]  [KEY_RETURN    ][KEY_ESC]
[KEY_LEFT_ARROW][KEY_DOWN_ARROW][KEY_LEFT_ARROW][' '    ]

In Mouse mode, the same directional keys control (Accelerated) mouse movement, 
Key 2: Left Mouse Button
Key 4: Right Mouse Button
Keys 5/6 : Scroll wheel up / down.

In Gamepad mode the directional keys represent a D-Pad, keys are otherwise the equivalent of:

[X]    [up]   [Y]  [B]
[left][down][right][A]

Key array is scanned every 5ms, there's a 5 iteration debounce on the keys, key state has to be steady for those iterations (i.e. 25ms as currently written) before an actual key press is registered. 
