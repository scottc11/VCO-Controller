
let resIn, resFb, gain;

/*
inverting amplifier --> Gain = Rf/Rin
non-inverting amplifier --> Gain = 1 + Rf/Rin
*/


gain = 4;
resFb = 100000; // 100k

resIn = (1+resFb)/gain;

console.log(resIn);