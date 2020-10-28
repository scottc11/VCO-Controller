#ifndef __PITCH_FREQUENCIES_H
#define __PITCH_FREQUENCIES_H

// Notes are separated by "semitone" intervals. 
// There are 12 seimtones in each octave, and fundamental frequencies are logarithmically spaced, 
// so the each note fundamental frequency is 2(1/12) = 1.0595 times the previous frequency.
// 1145 * .9405 == 

extern const float PITCH_FREQ[59] = {
  130.81,  //  C3
  138.59,  //  C#3/Db3
  146.83,  //  D3
  155.56,  //  D#3/Eb3	
  164.81,  //  E3	
  174.61,  //  F3	
  185.00,  //  F#3/Gb3
  196.00,  //  G3
  207.65,  //  G#3/Ab3
  220.00,  //  A3
  233.08,  //  A#3/Bb3
  246.94,  //  B3
  261.63,  //  C4
  277.18,  //  C#4/Db4
  293.66,  //  D4
  311.13,  //  D#4/Eb4
  329.63,  //  E4
  349.23,  //  F4
  369.99,  //  F#4/Gb4
  392.00,  //  G4
  415.30,  //  G#4/Ab4
  440.00,  //  A4
  466.16,  //  A#4/Bb4
  493.88,  //  B4
  523.25,  //  C5
  554.37,  //  C#5/Db5
  587.33,  //  D5
  622.25,  //  D#5/Eb5
  659.26,  //  E5
  698.46,  //  F5
  739.99,  //  F#5/Gb5
  783.99,  //  G5
  830.61,  //  G#5/Ab5
  880.00,  //  A5
  932.33,  //  A#5/Bb5
  987.77,  //  B5
  1046.50, //  C6
  1108.73, //  C#6/Db6
  1174.66, //  D6
  1244.51, //  D#6/Eb6
  1318.51, //  E6
  1396.91, //  F6
  1479.98, //  F#6/Gb6
  1567.98, //  G6
  1661.22, //  G#6/Ab6
  1760.00, //  A6
  1864.66, //  A#6/Bb6
  1975.53, //  B6
  2093.00, //  C7
  2217.46, //  C#7/Db7
  2349.32, //  D7	
  2489.02, //  D#7/Eb7	
  2637.02, //  E7	
  2793.83, //  F7	
  2959.96, //  F#7/Gb7	
  3135.96, //  G7	
  3322.44, //  G#7/Ab7	
  3520.00, //  A7	
  3729.31, //  A#7/Bb7	
  // 3951.07, //  B7	
  // 4186.01, //  C8	
  // 4434.92, //  C#8/Db8	
  // 4698.64, //  D8	
  // 4978.03, //  D#8/Eb8
};

extern const float FREQ_MAP[32][3] = {
  { 130.81,  138.59,  146.83 },   //  C3  C#3/Db3   D3
  { 155.56,  164.81,  174.61 },   //  D#3/Eb3  E3	 F3	
  { 185.00,  196.00,  207.65 },   //  F#3/Gb3  G3  G#3/Ab3
  { 220.00,  233.08,  246.94 },   //  A3         A#3/Bb3  B3
  { 261.63,  277.18,  293.66 },   //  C4        C#4/Db4   D4
  { 311.13,  329.63,  349.23 },   //  D#4/Eb4     E4      F4
  { 369.99,  392.00,  415.30 },   //  F#4/Gb4     G4     G#4/Ab4
  { 440.00,  466.16,  493.88 },   //  A4        A#4/Bb4   B4
  { 523.25,  554.37,  587.33 },   //  C5       C#5/Db5    D5
  { 622.25,  659.26,  698.46 },   //  D#5/Eb5     E5      F5
  { 739.99,  783.99,  830.61 },   //  F#5/Gb5     G5    G#5/Ab5
  { 880.00,  932.33,  987.77 },   //  A5        A#5/Bb5   B5
  { 1046.50, 1108.73, 1174.66 },  //  C6       C#6/Db6    D6
  { 1244.51, 1318.51, 1396.91 },  //  D#6/Eb6    E6       F6
  { 1479.98, 1567.98, 1661.22 },  //  F#6/Gb6    G6     G#6/Ab6
  { 1760.00, 1864.66, 1975.53 },  //  A6      A#6/Bb6    B6
};





#endif