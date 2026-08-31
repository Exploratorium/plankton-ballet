# Plankton Ballet

`plankton-ballet.ino` is an Arduino sketch for a museum exhibit built around a
Controllino Mini. A rotary encoder moves a blue light along a mirrored NeoPixel
display. The light moves smoothly toward the encoder's target position, leaving
the first and last 20 pixels outside the active area.

## Behavior

The sketch has two modes, selected by `CONTROLLINO_A0`:

- **Visitor mode** (`HIGH`): the encoder controls the blue light. The display
	uses a dim red background and the dot is rendered with sub-pixel blending.
- **Maintenance mode** (`LOW`): the complete display is full white and the
	bubbler output is switched on.

When the sketch starts, it prints its name and compile date/time to Serial,
initializes the hardware, and runs a two-second blue-dot sweep from one end of
the active area to the other and back. The bubbler output starts off and is
turned off again when returning from maintenance mode to visitor mode.

## Hardware

- **Controller:** Controllino Mini, ATmega328P at 5 V
- **Encoder:** `CONTROLLINO_IN0` and `CONTROLLINO_IN1` (Arduino interrupt pins
	2 and 3 on the target board)
- **NeoPixels:** data on `CONTROLLINO_D6`
- **Mode input:** `CONTROLLINO_A0`
- **Bubbler output:** `CONTROLLINO_D0`

The sketch creates a NeoPixel strip with `PIXEL_COUNT * 2` LEDs: 210 pixels in
each mirrored half, for 420 addressed LEDs in total. Each pixel in the first
half is also written to its mirrored position in the second half.

## Configuration

The main settings are at the top of `plankton-ballet.ino`:

| Setting | Value | Purpose |
| --- | ---: | --- |
| `PIXEL_COUNT` | `210` | Pixels in each mirrored half |
| `SUBPIXEL_SCALE` | `16` | Position units per physical pixel |
| `MAX_PIX_PER_SEC` | `20.0` | Maximum movement speed |
| `START_DEADBAND_PIXELS` | `20` | Pixels excluded at the start |
| `END_DEADBAND_PIXELS` | `20` | Pixels excluded at the end |
| `SUBPIX_PER_COUNT` | `16` | Encoder movement per quadrature count |

The active dot range is therefore pixels 20 through 189. The blue foreground
color is defined by `ACTIVE_R/G/B`, and the dim red background by `BG_R/G/B`.

## Building and uploading

Open `plankton-ballet.ino` as an Arduino sketch and select the appropriate
Controllino Mini board and serial port in the Arduino IDE. The sketch depends
on these libraries, included in the repository under `libraries/`:

- `Adafruit_NeoPixel`
- `CONTROLLINO`
- `Encoder`

After uploading, open Serial Monitor at **9600 baud** to see the sketch name
and compile timestamp.

## Repository layout

```text
plankton-ballet/
├── plankton-ballet.ino
├── readme.md
└── libraries/
		├── Adafruit_NeoPixel/
		├── CONTROLLINO/
		└── Encoder/
```
