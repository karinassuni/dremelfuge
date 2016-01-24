Parts List
=====================================
- Arduino UNO model R3
- Dremel model 7700, 7.2V MultiPro

- Protoboard for motor circuit
- Breadboard for potentiometer, push button, display circuits
- "Gorilla Glue" Epoxy, for gluing acrylic
- Hot glue
 
- [] Hitachi LCD
- [] LED Push Button, for changing modes
- [] Potentiometer, for setting time and speed
- Small potentiometer, for adjusting LCD contrast

- Transistor [NTE24]
- 5x 0.1 uF (bypass/filter/decoupling) capacitor, ceramic [NTE CML104M50, from Fry's]
  - 1 to denoise the potentiometer
  - 3 to absorb noise created by the motor, so it does not interfere with the LCD
    - https://www.pololu.com/docs/0J15/9
- 2x 1000 uF capacitors [Fry's brand]
  - 1 from the motor's power supply to the emitter
    - to prevent noise generated by motor's inductance from getting to other parts of the circuit
    - to dampen noise (fluctuating voltage/current) experienced by the motor
    - to help smoothen the voltage spikes from turning the motor on and off
    - http://robotics.stackexchange.com/questions/267/why-are-capacitors-added-to-motors-in-parallel-what-is-their-purpose
- Diode [1N5408]
  - rated to handle twice the max load current AND twice the negative of the max load voltage (lower than breakdown voltage), preventing the motor's back-EMF from destroying other components
  - must also have a maximum forward voltage low enough so that small load currents can make it past the diode, for when the smallest controlling currents are supplied at base
  - http://electronics.stackexchange.com/questions/95140/purpose-of-the-diode-and-capacitor-in-this-motor-circuiit

