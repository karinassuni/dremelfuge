Dremelfuge
========================================
A cheap Dremel-centrifuge!
<br>
<br>

## Agenda
- ~~Install Dremel motor and its power supply~~
- ~~Insert diode~~
- Troubleshoot motor circuit--battery smoke, insufficient current and rpm, lack of PWM control
- Solder all capacitors:
  - ~~to prevent motor noise to from producing LCD garbage~~
  - ~~to eliminate noise affecting potentiometer~~
  - to dampen noise experienced by motor, soldered according to https://www.pololu.com/docs/0J15/9
- Cut and hot-glue everything into case

========================================

#### Notes:
- No resistor needed--NTE24 transistor strong enough to accept raw PWM at base, (5\*(x/255)) Volts
- Choosing appropriate diodes and capacitors is simple and straightforward--only a few parameters matter
  - Diodes
    - Forward voltage (V\_F) - the maximum Volts at which the diode will be an "on" state and act like an open circuit (like it's not even there)
    - Maximum reverse voltage (V\_RRM) - the maximum negative voltage that the diode can handle
  - Capacitors
    - Capacitance (C)
    - Rated DC voltage (V\_R)

### Odd behaviors:
- Motor does not turn on again if it's toggled TOO QUICKLY
  - => back EMF not filtered well, motor circuit needs time to dissipate energy after a toggle?
- Motor humming but not spinning && motor won't start without a push
  - Must use a PWM voltage that's at least 25% of the motor's full speed voltage
  - Only works for FIXED pulse-width modulated voltage:
    - ~~=> capacitor linearizes the PWM square wave (turns PWM into an actually constant voltage) => constant current => no disconnected "spurts of torque"~~
    - ~~http://electronics.stackexchange.com/questions/27581/motor-hums-arduino-pwm~~
- Garbage on LCD
  - => bypass/filter/decoupling capacitor across the LCD, AND ANOTHER across the motor
  - http://www.seattlerobotics.org/encoder/jun97/basics.html
- Noise on potentiometer
  - => bypass/filter/decoupling cap
