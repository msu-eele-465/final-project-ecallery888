# Final project proposal

- [x] I have reviewed the project guidelines.
- [x] I will be working alone on this project.
- [x] No significant portion of this project will be (or has been) used in other course work.

## MODIFICATION
I have switched the temperature sensor out for an infrared sensor on Trevor's suggestion.

## Embedded System Description

My embedded system is a trap door meant for inclusion in a wealthy villain's secret lair, outside of a bank vault, or at the entrance to a doomsday bunker. It'll sense the presence of a person outside the locked door and prompt them to enter the passcode and hit a hidden button in a certain period of time. If they fail, a motor will drive the trap door and they will be sent plummeting into a prison cell that will keep them locked away until the owner comes back.

**The inputs to the system will be:**
1.  A button: This button will need to be pressed before entering the passcode to enable keypad access. If it is not pressed before an attempt is made to enter the passcode, the trapdoor will trigger.
2.  Keypad: The person trying to enter the passcode will have to do so within a limited amount of time into the 4x4 membrane keypad.
3.  Temperature sensor (LM19): The temperature sensor will be used to determine if a person is at the pad. The person must put their hand on the sensor so it can detect the body heat and then it will display a message that the keypad is ready to be used. eCOMP(a software module) will be used to trigger keypad activation when the temperature rises above a certain level.

**The outputs of the system will be:**
1. LCD display: This will display instructions for whoever is attempting to enter the room.
2. Buzzer: This buzzer will trigger when someone falls through the trapdoor, alerting anyone nearby.
3. Motor: This motor will turn, moving the theoretical trapdoor with gears to open and then close again (I attached a wheel to it to better show it spinning).
4. LED(s): There will be a status LED or two as well as one that turns on when the keypad is ready for entry.

## Hardware Setup

I'll require a servo motor, a buzzer, the MSP430s, the LM19, and the LCD display, as well as a power supply to power the motor. Below is a simple circuit block diagram illustrating how each element will be connected.

![alt text](../assets/FinalProjectCharts-ConceptualDiagram.svg)

## Software overview

My master will communicate to the slaves via I2C, reading the temperature level using eCOMP to determine when to begin operations. Once the tempurature is high enough, the system will start up and go through the steps outlined below.

![alt text](../assets/FinalProjectCharts-FlowchartSimple.svg)

## Testing Procedure

The project will be tested by a simulated encounter. Although there won't be an actual trapdoor, the servo spinning a wheel will show how it would spin the gears needed to operate a trapdoor.

## Prescaler

Desired Prescaler level: 

- [x] 100%
- [ ] 95% 
- [ ] 90% 
- [ ] 85% 
- [ ] 80% 
- [ ] 75% 

### Prescalar requirements 

**Outline how you meet the requirements for your desired prescalar level**

**The inputs to the system will be:**
1.  Button
2.  Keypad
3.  Temperature sensor (LM19)

**The outputs of the system will be:**
1. LCD display
2. Buzzer
3. Motor
4. LED(s)

**The project objective is**

Design a security system to protect one's house from criminals and catch any that may appear.

**The new hardware or software modules are:**
1. Servo motor
2. eCOMP

The Master will be responsible for:

The general state of the system, interfacing with the slaves and keeping everything synced.

The Slave(s) will be responsible for:

1 slave MSP43FR2310 will drive the LCD and another will drive the motor/buzzer.


### Argument for Desired Prescaler

This project clearly has 7 combined inputs/outputs, uses master/slave topology, and has a "real objective". eCOMP (can compare read values with internal reference voltages, use ISRs as well as set outputs) has never been used before in either 371 or 465, and the servo motor I'm using is different from the stepper motor that we used in 371. It also is relatively complex to implement as it requires an external power supply, precise tuning and driving, and care to not overexert the motor. Given that I've adapted 2 relatively complex hardware/software modules, I believe it fulfills the final requirement.
