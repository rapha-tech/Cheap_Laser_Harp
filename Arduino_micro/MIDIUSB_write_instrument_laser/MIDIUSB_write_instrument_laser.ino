/*
 * MIDIUSB_test.ino
 *
 * Created: 4/6/2015 10:47:08 AM
 * Author: gurbrinder grewal
 * Modified by Arduino LLC (2015)
 */ 

#include "MIDIUSB.h"

#define BUTTON_PIN_0 8
#define BUTTON_PIN_1 9
#define BUTTON_PIN_2 5
#define BUTTON_PIN_3 6
#define BUTTON_PIN_4 7
#define BUTTON_PIN_5 4

int buttons_pins[6] = {BUTTON_PIN_0, BUTTON_PIN_1, BUTTON_PIN_2, BUTTON_PIN_3, BUTTON_PIN_4, BUTTON_PIN_5};
int old_states[6] = {0, 0, 0, 0, 0, 0};
int notes[] = {0, 1, 2, 3, 4, 5};

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void setup() {
  Serial.begin(115200);
  for(int i = 0; i<6; i++)
    pinMode(buttons_pins[i], INPUT_PULLUP);
}

void loop() {

  // check buttons
  for(int i = 0; i<6; i++)
  {
    
    int state = digitalRead(buttons_pins[i]);
    if(state != old_states[i])
    {
      old_states[i] = state;
      if(state == 0)
      {
        noteOn(0, notes[i], 127);   // Channel 0, middle C, normal velocity
      }
      else
      {
        noteOff(0, notes[i], 127);  // Channel 0, middle C, normal velocity
      }
      MidiUSB.flush();
      delay(10);
    }
  }
}
