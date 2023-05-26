
## Introduction

Adast Maxima MS80 is a paper (paperboard) cutting machine (guillotine) produced in the Czech Republic in the early 1990s, still in operation in Vilnius, Lithuania, 2023.

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

The machine still does the job even now (May 2023).

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

## Problem Setting

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

The microcontroller did not survive, so we had to make a new circuit board from what was salvaged. The exact operating regimes and work flows were lost, but the main function, i.e. precise electronic guillotine (knife) positioning and cutting of a paper, was retained.

Remarkably, there exists a commercial solution/middleware designed to tackle this very specific problem, see i.e. [PD-04][1] which provides a rotary encoder along with the microcontroller based circuit board with the program designed for paper cutting machines. The PD-04 system is expensive. It costs [around 1220 euro (2023)](https://www.en.chip-elektronika.pl/readers-programmers-for-paper-cutters/control-system-pd-04/) for the hardware alone, and it takes two men (mechanics and electrical engineer) and several days to install it. This did not play well with the risks. Nobody knew for sure if PD04 would work on this particular old Adast Maxima, without the need to replace the motor and some unique electrical relays as well.

## Adast Maxima MS80

The machine operation breaks into the three stages:

1. The operator enters the distance value.
2. The machine repositions its guillotine.
3. The operator triggers the cutting.

There are a few other modes, but they are not essential. The position is sensed via a rotary encoder which, along with the guillotine, is shown in the photos below. The fourth picture shows a disk-like handle used to manually exit the emergency situations (e.g. lift the guillotine up if it gets stuck during the cutting as the electricity disappears).

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

## Code

The code is written for the [ATmega16/32][4] microcontroller and the avr-gcc compiler. The chip has 32 IO pins which we considered plenty. This turned out to be a very tight choice. It would have been better to apply a ready-made array of 7-segment LED indicators with a keypad, both based on the 2 wire/I2C interfaces, in order to reduce soldering effort. However, we wanted to match the board with the specific digit indicators and the factory panel.

The code implements the three phases: (i) initial calibration, (ii) target distance setup, and (iii) knife motion/positioning. There are all sorts of minor complications that probably do not deserve a textual description here, one should simply see the code. 
  
Initially, when turning the machine on, the guillotine's position is unknown and it is located where it has been left when the machine has been switched off. There is no distance value to read from the permanent memory, nor such a method would be viable. All we know is that: (i) the guillotine is somewhere in between the starting marker (approx. 20mm) and the end wall (approx. 810mm), (ii) there is a sensor located approximately at the 225mm from the starting marker, and (iii) the sensor sends the most precise signal only when the guillotine passes the sensor from the forward direction, that is when moving from the end towards the start.

Since the position of the knife is unknown initially, and we do not know in which direction its only calibrating sensor will be reached, an operator needs to see a rough location of a knife and move it manually so that the knife passes the sensor. This is not as automatic or convenient as it should be, but it is the price we pay for having only a single distance-calibrating sensor.

The code may resync the distance value whenever a knife passes the sensor, but this is commented out. There are options to position the knife so that it first passes the target value by 5cm and then comes back, supposedly to always make the final stop from the forward direction.

## Precision 

The loss of precision can happen in two distinct ways: (i) in a cumulative/catastrophic loss, and (ii) as a constant bias which can be alleviated.

The first category includes the single most critical parameter of this system, i.e. the distance traveled per encoder impulse, in microns (MPS). This value must be known precisely, otherwise the error will accumulate in time. The encoder specification states that MPS=40mu, and we halve this value by reading the AB-states of the rotary encoder in a certain manner. However, we do not know the MPS deviation and whether the original system has not been corrected in code, via some precision/laser measurement.

Let MPS = 40mu. After 10K impulses the distance becomes 40mu x 10000 = 40cm.

Suppose that the real value MPS = 40.1mu. Then, 40.1mu x 10000 = 40.1cm. One tenth of a micron accumulates into the whole millimeter, and this is just a single 40cm. movement. After ten such movements, the error will be 1cm!

This example shows that a standard ruler with a millimeter scale and the movement of 10000 impulses can determine only the first decimal part of the micron. Given the millimeter scale, we need to measure longer distances, e.g. 100K-impulse motion leading to, say, 400.4cm would produce MPS = 400.4cm/1e5 = 40.04mu. This is not possible to do, unfortunately, since the maximal guillotine travel is about 80cm. We could use a more precise caliper, but typically its maximal range will only be 4cm and the accuracy of 0.1mm, which gives us nothing.

Another way towards greater accuracy is to increase the impulse numbers. The quadrature encoder outputs two binary signals A and B which produce four states per cycle. These states can be counted, increasing the number of impulses and hence resolution 4x. This is still insufficient, but consider how much the number of impulses matters with the following example. Suppose a guessed MPS=40mu and we measure the distance traveled with only 25 generated impulses. This is about 1mm of travel, which, when measured with a millimeter accuracy, leads to 100% error. Whereas in the example above with 10K impulses, the relative error will be (40.1-40.0)/40 = 0.25%.  

Therefore, we cannot determine MPS with an adequate precision by means of a mm-ruler. We can only verify that the integral micron part is 40mu and see if the decimal part is close to zero, which seems to be the case.

The second group of errors add a constant bias to the distance value. For instance, the sensor position is known only approximately with a millimeter precision of a standard ruler which might introduce a mm/sub-mm bias in the real and displayed knife position. The bias is constant though, so it does not accumulate into the cascaded loss of precision with each knife movement; it can be measured and accounted for, in code. A similar case holds for the potentially imprecise stopping distance values. The only requirement/hope is that they are constant, which is the case only with a single speed and distances larger than a few centimeters. Stopping/braking distance depends on the direction, which is accounted for in the code. 

## Additional Remarks

- The machine is a technological marvel of precision mechanics, hydraulics, electrical relay engineering, and the microprocessor logic.

- Repairing a microcontroller unit (MCU) based board will seldom be economically viable. 
  Rewriting a microcontroller program demands rediscovering bits of the original R&D, which takes time, but the benefit of scaling is lost. 
  The result is just one repaired device, not many of them sold after the original R&D.

- The Polish PD04 "middleware" is a clever transformation of one such repairing process to a commercial product.

- The machine is surprisingly long lasting. We fixed it on February 2020, and it worked without a major hassle until May 2023. Recently, the knife-lifting hydraulics had to be replaced, and we suspect there is also something with the motor brake system that occasionally refuses to move the knife. This problem becomes significant only with very high loads and it is not as bad as it sounds since the impulses are counted correctly and the knife positioning remains precise. It only somewhat annoys the operator as at those "motion refusal" times the operator needs to re-enter the same target distance value again.

- In the nearest future a factory owener plans to do a major maintenance of the motor. If that does not fix the high load problem, we might replace the motor and also add the inverter which could allow a very smooth knife motion, but the code will need to be rewritten. TBC...
  
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
