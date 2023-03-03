
## Introduction

Adast Maxima MS80 is a paper (paperboard) cutting machine (guillotine) produced in Czechoslovakia in 1980s, still in operation in Vilnius, Lithuania, 2022.

<table>
<tr>
<th style="text-align:center"> Adast Maxima MS80 under Repair</th>
</tr>
<tr>
<td>
<img src="./images/adastMain.jpg"  alt="Adast Maxima MS80 under Repair" width="100%" >
</td>
</tr>
</table>

The repair was executed jointly by me and Saulius Rakauskas in about one week in February 2020. He disassembled the machine, designed a new circuit board, did all the soldering and hardware testing. I wrote the C program for the ATmega16 microcontroller with which we replaced the original Tesla chipset. 

The machine still does the job even now (the end of 2022), but it is at the end of its life line due to worn mechanics.

<table>
<tr>
<th style="text-align:center"> Adast Maxima MS80 in Action</th>
</tr>
<tr>
<td>
<img src="./images/adastAction.jpg"  alt="Adast Maxima MS80 in Action" width="100%" >
</td>
</tr>
</table>

## Project Details

An accident due to the electric current overload in the factory burnt the main circuit board, see below. Its fusers went off, but the board died first.

<table>
<tr>
<th style="text-align:center"> Circuit Board: Front </th>
<th style="text-align:center"> Circuit Board: Back </th>
</tr>
<tr>
<td>

<img src="./images/adastFront.jpg"  alt="Adast electric circuit board burnt front" width="100%" >

</td>
<td>

<img src="./images/adastBack.jpg"  alt="Adast electric circuit board burnt back" width="100%" >

</td>
</tr>
</table>

The microcontroller did not survive, so we had to make a new circuit board from what was salvaged. The exact operating regimes and work flows were lost, but the main functionality, cutting the paper, was retained.

Remarkably, there exists a commercial solution/middleware designed to tackle this very specific problem, see i.e. [PD-04][1] which provides a microcontroller based circuit board with the program designed for paper cutting machines. The PD-04 system is expensive though ([around 1325 euro](https://www.en.chip-elektronika.pl/readers-programmers-for-paper-cutters/control-system-pd-04/)).

## Results

The machine operation breaks into the three stages:

1. The operator enters the distance value.
2. The machine repositions its guillotine.
3. The operator triggers the cutting.

There are a few other modes, but they are not essential. The position is sensed via a rotary encoder which, along with the guillotine, is shown in the photos below. The fourth picture shows a certain disk-like handle used to manually exit the emergency situations (e.g. to lift the guillotine up if it gets stuck during the cutting as the electricity disappears).

<table>
<tr>
<th> Guillotine </th>
<th> Rotary Encoder</th>
</tr>
<tr>
<td>

<img src="./images/adastKnife.jpg"  alt="Adast guillotine" width="100%" >

</td>
<td>

<img src="./images/adastCounterZoom.jpg"  alt="Adast encoder" width="100%" >

</td>
</tr>
</table>

<table>
<tr>
<th> Rotary Encoder </th>
<th> Deadlock Handling </th>
</tr>
<tr>
<td>

<img src="./images/adastCounterRemoved.jpg"  alt="Adast circuit diagram part 1" width="100%" >

</td>
<td>

<img src="./images/adastDeadlockHandle.jpg"  alt="Adast circuit diagram part 2" width="100%" >

</td>
</tr>
</table>

The initial distance synchronization deserves a few words. At first, the guillotine's position is unknown and it is located where it has been left when the machine has been switched off. There is no distance value to read from the permanent memory, nor such a method would be viable. All we know is that (i) the guillotine is somewhere in between the starting marker (approx. 20mm) and the end wall (approx. 810mm), (ii) there is a sensor located at the 225mm from the starting marker, and (iii) we hit the precise location of the guillotine only when it passes the sensor from one specific direction, i.e. from the starting position towards the end (a movement from the opposite direction also activates the sensor signal, but it yields a less precise value).

The guillotine is made of a heavy steel and thus has a large stopping distance that it still travels once the movement is signalled to stop. This stopping distance is measured to be constant, about 2100 microns when stopping by going in the forward direction, and 2400 microns when stopping backwards. It is doubtful that these values remain constant, but their precise knowledge is not critical as long as the rotary encoder senses (counts) every guillotine movement.

The worst part is that it seems to react to the motion commands with small occasional erroneous displacements that bypass the rotary encoder and accumulate in time, which eventually results in the loss of precision and the need to restart the machine.

The code is written for the [ATmega16][4] microcontroller and the avr-gcc compiler. The chip has 32 IO pins which we considered plenty. This turned out to be a very tight choice. It would have been better to apply a ready-made array of 7-segment LED indicators with a keypad, both based on the 2 wire/I2C interfaces, in order to reduce soldering effort. However, we wanted to match the board with the specific digit indicators and the factory panel.

## References

- [PD-04][1]
- [rotary-encoder][2]
- [vytassblog][3]
- [ATmega16][4]
- [A Tutorial on Portable Makefiles, 2017](https://nullprogram.com/blog/2017/08/20/)
- [A Tutorial on Portable Makefiles, Comments on Hacker News](https://news.ycombinator.com/item?id=32303193)

[1]: https://www.en.chip-elektronika.pl/readers-programmers-for-paper-cutters/control-system-pd-04/
[2]: https://howtomechatronics.com/tutorials/arduino/rotary-encoder-works-use-arduino/
[3]: http://blog.elektronika.lt/vytassblog/?page_id=113
[4]: https://components101.com/microcontrollers/atmega16-pinout-features-datasheet
