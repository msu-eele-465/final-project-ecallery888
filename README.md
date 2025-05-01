# EELE 465 Final Project: Trapdoor Security System

## Embedded System Description

My embedded system is a trap door meant for inclusion in a wealthy villain's secret lair, outside of a bank vault, or at the entrance to a doomsday bunker. It'll sense the presence of a person outside the locked door and prompt them to enter the passcode and hit a hidden button in a certain period of time. If they fail, a motor will drive the trap door and they will be sent plummeting into a prison cell that will keep them locked away until the owner comes back.

I used eCOMP to monitor my IR sensor for the extra difficult software/hardware module necessary for a 95%.

**System Inputs**
1.  A button, part of the steps to successfully avoid activating the trapdoor
2.  Keypad, for passcode entry
3.  IR Sensor, to detect IR waves

**System Outputs**
1. LCD display: This displays instructions for whoever is attempting to enter the room.
2. Buzzer: This buzzer triggers when someone falls through the trapdoor, alerting anyone nearby.
3. Motor: This motor turns, moving the theoretical trapdoor with gears to open and then close again (I attached a wheel to it to better show it spinning).
4. LED(s): There are numerous LEDs to indicate status and readiness.

##  Architecture

![Software architecture](docs/assets/FinalProjectCharts-SoftwareArchitecture.svg)

## Flowchart

![Flowchart](docs/assets/FinalProjectCharts-FlowchartSimple.svg)

## Circuit Diagram

![Circuit Diagram](docs/assets/FinalProjectCharts-CircuitDiagram.svg)

