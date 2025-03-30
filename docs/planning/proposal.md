# Final project proposal

- [x] I have reviewed the project guidelines.
- [x] I will be working alone on this project.
- [x] No significant portion of this project will be (or has been) used in other course work.

## Embedded System Description

My embedded system is a trap door meant for inclusion in a wealthy villain's secret lair, outside of a bank vault, or at the entrance to a doomsday bunker. It'll sense the presence of a person outside the locked door and prompt them to enter the passcode and hit a hidden button in a certain period of time. If they fail, a motor will drive the trap door and they will be sent plummeting into a prison cell that will keep them locked away until the owner comes back.

**The inputs to the system will be:**
1.  A button: This button will need to be pressed before entering the passcode to enable keypad access. If it is not pressed before an attempt is made to enter the passcode, the trapdoor will trigger.
2.  Keypad: The person trying to enter the passcode will have to do so within a limited amount of time into the 4x4 membrane keypad.
3.  Temperature sensor (RTC): The Real Time Clock's temperature sensor will be used to determine if a person is at the pad. The person must put their hand on the clock so it can detect the body heat and then it will display a message that the keypad is ready to be used.

**The outputs of the system will be:**
1. LCD display: This will display instructions for whoever is attempting to enter the room.
2. Buzzer: This buzzer will trigger when someone falls through the trapdoor, alerting anyone nearby.
3. Motor: This motor will turn, moving the theoretical trapdoor with gears to open and then close again (I attached a wheel to it to better show it spinning).
4. LED(s): There will be a status LED or two as well as one that turns on when the keypad is ready for entry.

The system will run on a MSP430FR2355 and use 1 slave MSP43FR2310 to drive the LCD and another to drive the motor/buzzer.

## Hardware Setup

What hardware will you require? Provide a conceptual circuit diagram and/or block diagram to help the reviewers understand your proposal. Be sure to introduce and discuss your figures in the text.

## Software overview

Discuss, at a high level, a concept of how your code will work. Include a *high-level* flowchart. This is a high-level concept that should concisely communicate the project's concept.

## Testing Procedure

Briefly describe how you will test and verify that your project is a success. Think about how you will *demo* the final project. If you need extra equipment for the demo, be sure that you can either bring the equipment or convincingly simulate it. For example, if you want to build a system that uses CAN bus to interface with your car, you won't be able to bring your car into Cobleigh for the demo...


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
3.  Temperature sensor (RTC)

**The outputs of the system will be:**
1. LCD display
2. Buzzer
3. Motor
4. LED(s)

**The project objective is**

{text – 1 to 2 sentences}

**The new hardware or software modules are:**
1. Servo motor
2. ICC or eCOMP


The Master will be responsible for:

{text – 1 to 2 sentences}

The Slave(s) will be responsible for:

{text – 1 to 2 sentences}



### Argument for Desired Prescaler

Consider the guidelines presented in the lecture notes and convince the reviewers that this proposal meets the minimum requirements for your desired prescale level.
