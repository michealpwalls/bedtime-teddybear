/**

    Arduino: Bedtime Teddybear
    Copyright (C) 2013 Micheal Walls (michealpwalls@gmail.com)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

*/

#include <FatReader.h>
#include <SdReader.h>
#include <avr/pgmspace.h>
#include "WaveUtil.h"
#include "WaveHC.h"

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the filesystem on the card
FatReader f;      // This holds the information for the file we're play

WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

#define DEBOUNCE 100  // button debouncer

// Button press counter variables
int state=LOW;      // Initial state of th ebutton
int lastState=LOW;  // Initial previous state of the button
int count=0;        // Press counter

// this handy function will return the number of bytes currently free in RAM, great for debugging!   
int freeRam(void)
{
  extern int  __bss_end; 
  extern int  *__brkval; 
  int free_memory; 
  if((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__bss_end); 
  }
  else {
    free_memory = ((int)&free_memory) - ((int)__brkval); 
  }
  return free_memory; 
} 

void sdErrorCheck(void)
{
  if (!card.errorCode()) return;
  putstring("\n\rSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  putstring(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}

void setup() {
  // set up serial port
  Serial.begin(9600);
  putstring_nl("WaveHC with 1 button");
  
  putstring("Free RAM: ");       // This can help with debugging, running out of RAM is bad
  Serial.println(freeRam());      // if this is under 150 bytes it may spell trouble!
  
  // Set the output pins for the DAC control. This pins are defined in the library
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
 
  // pin13 LED
  pinMode(13, OUTPUT);
  
  // pin6 button input
  pinMode(6, INPUT);
  
  // Set the state to button's input
  state=digitalRead(6);
 
  // enable pull-up resistor on switch pin (analog input)
  digitalWrite(6, HIGH);
 
  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)  
    putstring_nl("Card init. failed!");  // Something went wrong, lets print out why
    sdErrorCheck();
    while(1);                            // then 'halt' - do nothing!
  }
  
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);
 
// Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {     // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                             // we found one, lets bail
  }
  if (part == 5) {                       // if we ended up not finding one  :(
    putstring_nl("No valid FAT partition!");
    sdErrorCheck();      // Something went wrong, lets print out why
    while(1);                            // then 'halt' - do nothing!
  }
  
  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(),DEC);     // FAT16 or FAT32?
  
  // Try to open the root directory
  if (!root.openRoot(vol)) {
    putstring_nl("Can't open root dir!"); // Something went wrong,
    while(1);                             // then 'halt' - do nothing!
  }
  
  // Whew! We got past the tough parts.
  putstring_nl("Ready!");
}

void loop() {
  // Initially, we don't do anything until the button is pressed for the first time
  if (state==HIGH && lastState==LOW){
    // The button has been pressed, so we increment the counter.
    ++count;
    /**
    // If a song was playing when the button was pressed, stop it.
    if(wave.isplaying) {
      wave.stop();
    }
    */

    // Our button has been pressed once so we begin the 5 minutes of music
    if(count == 1) {
      // playcomplete has been modified to return true if a button has not been pressed during play
      // and false if a button was pressed, interrupting the song
      if(playcomplete("YOUGOT~1.WAV")) {
        playcomplete("TEDDYB~1.WAV");
      } else {
        // The button interrupted the song, so we increment press counter and begin 10 minutes of music
        count = 2;
        // If the song was not interrupted by button press, do nothing
        if(playcomplete("LULLAB~1.WAV")) {
          // Nothing to do here
        } else {
          // A 3rd button was pressed, interrupting the music. We reset the counter to clean up.
          count = 0;
        }
      }
    // Our button has been pressed twice so we begin 10 minutes of music
    } else if(count == 2) {
      if(playcomplete("LULLAB~1.WAV")) {
        //Nothing to do here
      } else {
        // The button was pressed a 3rd time, interrupting the music
        count = 3;
      }
    // Our button has been pressed three times, so we stop whatever's playing and clean up.
    } else if(count == 3) {
      if(wave.isplaying) {
        wave.stop();
      }
      count = 0;
    }

  }

  lastState=state;
  state=digitalRead(6);
}

// Plays a full file from beginning to end with no pause.
boolean playcomplete(char *name) {
  // call our helper to find and play this name
  playfile(name);
  while (wave.isplaying) {
    // Update the button state
    lastState=state;
    state=digitalRead(6);

    // Interrupt the song if the button is pressed
    if (state==HIGH && lastState==LOW){
      wave.stop();
      return false;
    }
  }
  return true;
  // now its done playing
}

void playfile(char *name) {
  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  if (!f.open(root, name)) {
    putstring("Couldn't open file "); Serial.print(name); return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV"); return;
  }
  
  // ok time to play! start playback
  wave.play();
}
