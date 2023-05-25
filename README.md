
## Introduction

Adast Maxima MS80 is a paper (paperboard) cutting machine (guillotine) produced in Czechoslovakia in 1980s, still in operation in Vilnius, Lithuania, 2023.

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

The machine still does the job even now (2023), but it is at the end of its life time. 

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

The microcontroller did not survive, so we had to make a new circuit board from what was salvaged. The exact operating regimes and work flows were lost, but the main functionality, cutting the paper, was retained.

Remarkably, there exists a commercial solution/middleware designed to tackle this very specific problem, see i.e. [PD-04][1] which provides a rotary encoder along with the microcontroller based circuit board with the program designed for paper cutting machines. The PD-04 system is likely reasonably priced ([around 1220 euro](https://www.en.chip-elektronika.pl/readers-programmers-for-paper-cutters/control-system-pd-04/)), but there are also replacement work costs, and this did not play well with the risks. Nobody knew for sure if PD04 would work on this particular old Adast Maxima. In addition, nobody wants to invest in a machine that it is at the end of its life time which could be the next week or next month.

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

The code is written for the [ATmega16][4] microcontroller and the avr-gcc compiler. The chip has 32 IO pins which we considered plenty. This turned out to be a very tight choice. It would have been better to apply a ready-made array of 7-segment LED indicators with a keypad, both based on the 2 wire/I2C interfaces, in order to reduce soldering effort. However, we wanted to match the board with the specific digit indicators and the factory panel.

The code is split into three folders:

* 01 Microns per Step

  This program simply moves the knife forward/backward and shows the corresponding impulse counter by the encoder. The coordinate system is reversed, i.e. in the forward mode the counter is set to 9990 and is then decremented, while in the backward mode the counter starts at zero and goes up. We use a traditional mm-scale ruler to manually measure the knife displacement between two arbitrary points A and B, which can then be divided by the impulses counted, yielding microns per step (MPS). See below some further comments on precision.
  
* 02 Breaking Distance in Steps

  The guillotine is made of heavy steel and thus has a large stopping distance that it still travels once the movement is signalled to stop. This stopping distance needs to be measured and the code in this folder serves this purpose. One initiates the knife motion in either direction that takes 5000 impulses, the program then issues the stop signal, yet the knife will still travel a certain distance. Its precise value can be deduced by knowing an MPS value and reading the extra number of impulses generated by the encoder during the stopping.
  
* 03 Final Program   
  
  The final code implements three phases: (i) initial calibration, (ii) target distance setup, and (iii) knife motion/positioning. There are all sorts of minor complications that probably do not deserve a textual description here, one should simply see the code. 
  
Adast Maxima MS80 is peculiar in that it does not have the sensors to signal when the knife reaches the edges. Initially, when turning the machine on, the guillotine's position is unknown and it is located where it has been left when the machine has been switched off. There is no distance value to read from the permanent memory, nor such a method would be viable. All we know is that: (i) the guillotine is somewhere in between the starting marker (approx. 20mm) and the end wall (approx. 810mm), (ii) there is a sensor located approximately at the 225mm from the starting marker, and (iii) the sensor sends the most precise signal only when the guillotine passes the sensor from the forward direction, that is when moving from the end towards the start.

Since the position of the knife is unknown initially, and we do not know in which direction its only calibrating sensor will be reached, an operator needs to see a rough location of a knife and move it manually so that the knife passes the sensor. This is not as automatic or convenient as it should be, but it is the price we pay for having only a single distance-calibrating sensor.

The code also resyncs the distance value whenever a knife passes the sensor. There are options to position a knife so that it first passes the target value by 5cm and then comes back, supposedly to always make the final stop from the forward direction.

## Precision 

The loss of precision can happen in two distinct ways: (i) as a cumulative/catastrophic precision loss, and (ii) as a constant bias which can be alleviated.

The first category includes the single most critical parameter of this system, i.e. the distance traveled per encoder impulse, in microns (MPS). This value must be known precisely, otherwise the error will accumulate in time. The encoder specification states that MPS=40mu, but we do not know its deviation and whether the original system has not been corrected in code, via some precision/laser measurement which we cannot perform.

Let MPS = 40mu. After 10K impulses the distance becomes 40mu x 10000 = 40cm.

Suppose that the real value MPS = 40.1mu. Then, 40.1mu x 10000 = 40.1cm. One tenth of a micron accumulates into the whole millimeter, and this is just a single 40cm. movement. After ten such movements, the error will be 1cm!

This example shows that a standard ruler with a millimeter scale and the movement of 10000 impulses can determine only the first decimal part of the micron. Given the millimeter scale, we need to measure longer distances, e.g. 100K-impulse motion leading to, say, 400.4cm would produce MPS = 400.4cm/1e5 = 40.04mu. This is not possible to do, unfortunately, since the maximal guillotine travel is about 80cm. We could use a more precise caliper, but typically its maximal range will only be 4cm and the accuracy of 0.1mm, which gives us nothing.

Another way towards greater accuracy is to increase the impulse numbers. The quadrature encoder outputs two binary signals A and B which produce four states per cycle. These states can be counted, increasing the number of impulses and hence resolution 4x. This is still insufficient, but consider how much the number of impulses matters with the following example. Suppose a guessed MPS=40mu and we measure the distance traveled with only 25 generated impulses. This is about 1mm of travel, which, when measured with a millimeter accuracy, leads to 100% error. Whereas in the example above with 10K impulses, the relative error will be (40.1-40.0)/40 = 0.25%.  

Therefore, we cannot determine MPS with an adequate precision by means of a mm-ruler. We can only verify that the integral micron part is 40mu and see if the decimal part is close to zero.

The second group of errors add a constant bias to the distance value. For instance, the sensor position is known only approximately with the mm precision of a standard ruler which might introduce a mm/sub-mm bias in the real and displayed knife position. The bias is constant though, so it does not accumulate into the cascaded loss of precision with each knife movement; it can be measured and accounted for, in code. A similar case holds for the potentially imprecise stopping distance values. The only requirement/hope is that they are constant.

## Additional Remarks

- The machine is a technological marvel of precision mechanics, hydraulics, large electric current engineering, and the microprocessor logic.

- Repairing a microcontroller unit (MCU) based board will seldom be economically viable. 
  Rewriting a microcontroller program demands rediscovering bits of the original R&D, which takes time, but the benefit of scaling is lost. 
  The result is just one repaired device, not many of them sold after the original R&D.

- The Polish PD04 "middleware" is a clever transformation of one such repairing process to a commercial product.

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
