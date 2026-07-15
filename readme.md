# Plankton Ballet 🦐💙✨

## Todos:
- [ ] Update knob interaction (reverse direction [done], use current direction turning rather than delta) 
- [ ] Package libraries together (migrate to local reference)

-> END TIME: next thursday, HMG QC 

Delayed until after decisions/QC done
- [ ] On-board RTC for daytime/night time modes

Welcome to the  AI-made exhibit controller in the tank. 🤖⚡

This project powers an interactive museum experience where visitors steer a blue light with a dial and watch brine shrimp swarm toward it in real time. It is proudly AI-based and  **not yet quality-controlled at all**. 

## Experience Description 🎛️🔵

Visitors can change the position of a blue light within the plankton tank using a dial that moves it left or right. As the light source shifts, a busy cloud of brine shrimp follows the glow. The not-blue lights are dim red to provide contrast.


## Content Connection 🌊☀️

Even tiny animals like plankton respond to a symphony of environmental cues.

Brine shrimp are especially responsive to the wavelengths of light found near the ocean surface. In sunlit upper waters, they can find the algae they eat to survive. Even simple organisms can show surprisingly complex behavior that shapes whole ecosystems.

## Exhibit Behavior Summary 🧠🔧

- Encoder dial moves a blue light target across the tank display.
- NeoPixels render a  blue-dot effect that guides shrimp movement.
- Dead-band at the physical start and end keeps motion away from edge zones.
- At real night, the system switches the bubbler on and  all-white lighting to simulate daytime conditions for shrimp behavior.

## Hardware Notes 🛠️

- Controller: Controllino Mini
- Inputs: Encoder on CONTROLLINO_IN0 / CONTROLLINO_IN1 (pin header)
- LEDs: NeoPixel strip on CONTROLLINO_D6 (pin header)
- Relay: Night/day control on CONTROLLINO_D0 Relay
