#include <Bounce2.h>
#include <Encoder.h>

// Zynthian implements midi UI control using a configurable Master Channel
// UI actions are bound to midi notes, which can be set in the webconf
// I might experiment with porting this to using OSC, but that would be later.
const int masterChannel = 16;

// Switch pins
/* 
at the time of publishing, this code assumes you are using testing branch,
which has some new CUIA actions which implement short/bold/long timing.
upcoming v5 panel has twenty buttons, they are bound to Master Channel notes 90+.
pins should be added to the array in the same order as the CUIA button index. 
for Zynthian v4 panels, pin index 0-3 are the encoder switches and 4-7 are the extra four.
*/
// pin for index:            0   1   2   3   4   5   6   7
const byte buttonPins[] = { 23, 22, 21, 20, 19, 18, 17, 16 };
// count pins specified and initialize array of Bounce objects
const byte buttonCnt = sizeof(buttonPins);
Bounce button[buttonCnt] = Bounce(); 

// Rotary Encoders
/*
You might need to experiment with pin pair order, depending on your encoders. Pay 
attention to the notes on indexing above - encoder index has to be the same 
as its matching switch index. At the time of publishing, the CUIA actions 
for the Zynpots have NOT been added to the default CUIA list, but there is a
run of nine free notes starting at E1 which has space to put the eight 
actions required in a neat set of indices. Details in the readme.

IF YOU DO NOT BIND THE ENCODER ACTIONS SEQUENTIALLY, THIS CODE WILL NOT WORK.

E1 is note 16. If you bind sequentially from there in down-up order, 
ie. E1 (16) = enc0 down, F1 (17) = enc0 up, F#1 (18) = enc1 down, etc,
...then you will not need to alter this code at all.
If you start at a different note, you will need to change firstEncoderCuiaIndex.

IF YOU DO NOT BIND THE ZYNPOT CUIA ACTIONS SEQUENTIALLY:
You will need to significantly change the //zynpots section of loop() to
manually map each encoder's two actions (up/down) to the specific note that
that you have selected. I would like to make this more robust, but the fact
that users have the ability to bind things however they like makes it a
moving target, so it's unlikely that my default code will get better than this. 
It is theoretically possible for this code to retrieve the specific binds from
the zynthian using , but now that my thing works I probably won't put in the time. 
*/
int firstEncoderCuiaIndex = 16;
Encoder zynpots[4] = {
Encoder(2,3),   // pins for encoder 0
Encoder(4,5),   // pins for encoder 1
Encoder(8,9),   // pins for encoder 2
Encoder(6,7),   // pins for encoder 3
};

// Basically, if you setup your Zynthian's CUIA binds as described in the readme,
// you can ignore everything from here. If you don't, it gets fun for you.
int currentCoderValue[4] = {0};

void setup() {
  Serial.begin(9600);

  for (byte i = 0; i < 4; i++) {
    zynpots[i].write(0);
  }
    
  for ( byte i = 0; i < buttonCnt; i++ ) {
    button[i].attach( buttonPins[i], INPUT_PULLUP );
    button[i].interval(10);   // debounce time
    button[i].update();
  }
}

void loop() {

  // update buttons
  for ( byte i = 0; i < buttonCnt; i++ )   button[i].update();

  // process changed buttons
  for ( byte i = 0; i < buttonCnt; i++ ) {
    if (button[i].changed()) {
      int debVal = button[i].read();
      Serial.print("Button "); Serial.print(i); Serial.print(", sending note "); Serial.print(i+90);
      if (debVal == 0) {
        usbMIDI.sendNoteOn(i+90, 64, masterChannel);
        Serial.println(" on.");
      } else {
        usbMIDI.sendNoteOff(i+90, 0, masterChannel);
        Serial.println(" off.");
      }
    }    
  }

  // zynpots
  for ( byte i = 0; i < 4; i++ ) {
    currentCoderValue[i] = zynpots[i].read();
    if (currentCoderValue[i] >= 4) {
      Serial.print("Encoder "); Serial.print(i); Serial.println(" up"); 
      usbMIDI.sendNoteOn(i*2 + 16, 64, masterChannel);
      zynpots[i].write(0);
    } else if (currentCoderValue[i] <= -4) {
      Serial.print("Encoder "); Serial.print(i); Serial.println(" down"); 
      usbMIDI.sendNoteOn(i*2 + 17, 64, masterChannel);
      zynpots[i].write(0);
    } 
  }

  // MIDI Controllers should discard incoming MIDI messages.
  while (usbMIDI.read()) {
  }
}
