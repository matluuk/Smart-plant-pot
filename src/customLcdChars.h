#ifndef CUSTOMLCDCHARS_H
#define CUSTOMLCDCHARS_H

#include <arduino.h>
/* blank
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
*/

byte temp[8] = {
  B00100,
  B01010,
  B01010,
  B01010,
  B01110,
  B11111,
  B11111,
  B01110
};


byte drop[8] = {
  B00000,
  B00100,
  B00100,
  B01010,
  B10001,
  B10001,
  B10001,
  B01110
};

byte smile[8] = {
  B00000,
  B00000,
  B01010,
  B00000,
  B10001,
  B01110,
  B00000,
  B00000
};

/**********************************************
Happy face
***********************************************/

// byte faceBottomLeftHappy[8] = {
//   B00000,
//   B00000,
//   B00000,
//   B00000,
//   B11000,
//   B01111,
//   B00011,
//   B00000
// };

// byte faceBottomMiddleHappy[8] = {
//   B01110,
//   B01010,
//   B00000,
//   B00000,
//   B00000,
//   B11111,
//   B11111,
//   B00000
// };

// byte faceBottomRightHappy[8] = {
//   B00000,
//   B00000,
//   B00000,
//   B00000,
//   B00011,
//   B11110,
//   B11000,
//   B00000
// };

// byte faceTopLeftHappy[8] = {
//   B00000,
//   B00000,
//   B00000,
//   B00000,
//   B00001,
//   B00001,
//   B00001,
//   B00001
// };

// byte faceTopMiddleHappy[8] = {
//   B00000,
//   B00000,
//   B00000,
//   B00000,
//   B00000,
//   B00000,
//   B00000,
//   B00000
// };

// byte faceTopRightHappy[8] = {
//   B00000,
//   B00000,
//   B00000,
//   B00000,
//   B10000,
//   B10000,
//   B10000,
//   B10000
// };



byte faceTopLeftHappy[8] = {
  B00000,
  B00011,
  B00100,
  B01000,
  B01000,
  B10000,
  B10000,
  B10000
};

byte faceTopMiddleHappy[8] = {
  B11111,
  B00000,
  B00000,
  B00000,
  B10001,
  B10001,
  B10001,
  B00000
};

byte faceTopRightHappy[8] = {
  B00000,
  B11000,
  B00100,
  B00010,
  B00010,
  B00001,
  B00001,
  B00001
};

byte faceBottomLeftHappy[8] = {
  B10000,
  B10010,
  B10010,
  B01001,
  B01000,
  B00100,
  B00011,
  B00000
};

byte faceBottomMiddleHappy[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B00000,
  B00000,
  B11111
};

byte faceBottomRightHappy[8] = {
  B00001,
  B01001,
  B01001,
  B10010,
  B00010,
  B00100,
  B11000,
  B00000
};


/**********************************************
Thirsty face
***********************************************/

byte faceTopLeftThirsty[8] = {
  B00000,
  B00011,
  B00100,
  B01000,
  B01000,
  B10000,
  B10100,
  B10100
};

byte faceTopMiddleThirsty[8] = {
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B10001,
  B00000,
  B00000
};

byte faceTopRightThirsty[8] = {
  B01000,
  B11000,
  B10100,
  B10110,
  B01010,
  B00001,
  B00001,
  B00001
};

byte faceBottomLeftThirsty[8] = {
  B11010,
  B10100,
  B10000,
  B01000,
  B01000,
  B00100,
  B00011,
  B00000
};

byte faceBottomMiddleThirsty[8] = {
  B01110,
  B10001,
  B00000,
  B11111,
  B01010,
  B01010,
  B00100,
  B11111
};

byte faceBottomRightThirsty[8] = {
  B00001,
  B00001,
  B00001,
  B00010,
  B00010,
  B00100,
  B11000,
  B00000
};

/**********************************************
Wet face
***********************************************/

byte faceBottomLeftWet[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11000,
  B01111,
  B00011,
  B00000
};

byte faceBottomMiddleWet[8] = {
  B01110,
  B01010,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B00000
};

byte faceBottomRightWet[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00011,
  B11110,
  B11000,
  B00000
};

byte faceTopLeftWet[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00001,
  B00001,
  B00001
};

byte faceTopMiddleWet[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

byte faceTopRightWet[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B10000,
  B10000,
  B10000,
  B10000
};



#endif