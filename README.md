bedtime-teddybear - By Micheal Walls (michealpwalls@gmail.com)
===============================================================

  Arduino project utilizing a Wave Shield to create a Teddybear that sings Bedtime songs to children.



Technical Description
======================

  Written in ANSI C.
  A revision of the "PiSpeakHC" example sketch provided with the WaveHC library.

    1. Added a button press counter (Digital I/O 6)
    2. Revised playcomplete() to return boolean false if button pressed during playback, else true
    3. Plays a pre-determined series of wav files based on the button press counter



Example Usage
==============

  Press button 1 time:  Play 5 minutes of music.
  Press button 2 times: Play 10 minutes of music.
  Press button 3 times: Stop playback and reset counter to 0.

  
