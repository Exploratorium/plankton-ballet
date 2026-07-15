/*
 * Museum exhibit – NeoPixel strip controller
 * Controllino Mini (ATmega328P @ 5 V)
 *
 * Encoder:  CONTROLLINO_IN0 (INT0, pin 2), CONTROLLINO_IN1 (INT1, pin 3)
 * NeoPixel: CONTROLLINO_D6 PIN HEADER only – leave screw terminal unwired
 *
 * Boot sequence:
 *   1. NeoPixel: blue dot sweeps 0→end→0 over 2000 ms
 */

#include <SPI.h>
#include <Controllino.h>
#include <Encoder.h>
#include <Adafruit_NeoPixel.h>

// ── Configuration ────────────────────────────────────────────────────────────

const int PIXEL_COUNT = 215;
const int SUBPIXEL_SCALE = 16;
const float MAX_PIX_PER_SEC = 20.0f;
const int START_DEADBAND_PIXELS = 20;
const int END_DEADBAND_PIXELS = 20;

// Sub-pixels moved per quadrature count. 16 = 1 pixel per count.
// If the dot moves too fast, increase this value; too slow, decrease it.
// A 128-step/rev encoder in full quadrature gives 512 counts/rev.
const int SUBPIX_PER_COUNT = SUBPIXEL_SCALE;

// Blue-dot palette
const uint8_t ACTIVE_R = 0, ACTIVE_G = 0, ACTIVE_B = 255;
const uint8_t BG_R = 0, BG_G = 3, BG_B = 0;

// ── Hardware ─────────────────────────────────────────────────────────────────

#define NEOPIXEL_PIN CONTROLLINO_D6

Encoder knob(CONTROLLINO_IN0, CONTROLLINO_IN1);
Adafruit_NeoPixel strip(PIXEL_COUNT * 2, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ── State ─────────────────────────────────────────────────────────────────────

const int MAX_SUBPIX = (PIXEL_COUNT - 1) * SUBPIXEL_SCALE;
const int ACTIVE_MIN_SUBPIX = START_DEADBAND_PIXELS * SUBPIXEL_SCALE;
const int ACTIVE_MAX_SUBPIX = ((PIXEL_COUNT - 1) - END_DEADBAND_PIXELS) * SUBPIXEL_SCALE;

int currentSubpix = ACTIVE_MIN_SUBPIX;
long lastEncPos = 0;
unsigned long prevFrameMs = 0;
int targetSubpix = ACTIVE_MIN_SUBPIX;

// ── Rendering ─────────────────────────────────────────────────────────────────

void showAllWhite()
{
    for (int i = 0; i < PIXEL_COUNT * 2; i++)
        strip.setPixelColor(i, strip.Color(255, 255, 255));
    strip.show();
}

void setPixelBothStrips(int i, uint8_t r, uint8_t g, uint8_t b)
{
    strip.setPixelColor(i, strip.Color(r, g, b));
    strip.setPixelColor(PIXEL_COUNT * 2 - i - 1, strip.Color(r, g, b));
}

void showBlue(int sp)
{
    int px = sp / SUBPIXEL_SCALE;
    int rem = sp % SUBPIXEL_SCALE;

    for (int i = 0; i < PIXEL_COUNT; i++)
    {
        int bri;
        if (i == px)
            bri = SUBPIXEL_SCALE - rem / 2;
        else if (i == px + 1)
            bri = rem / 2;
        else if (i == px - 1 && rem < 8)
            bri = (8 - rem) / 2;
        else if (i == px + 2 && rem > 8)
            bri = (rem - 8) / 2;
        else
        {
            setPixelBothStrips(i, BG_R, BG_G, BG_B);
            continue;
        }

        int inv = SUBPIXEL_SCALE - bri;
        setPixelBothStrips(i,
                           (ACTIVE_R * bri + BG_R * inv) / SUBPIXEL_SCALE,
                           (ACTIVE_G * bri + BG_G * inv) / SUBPIXEL_SCALE,
                           (ACTIVE_B * bri + BG_B * inv) / SUBPIXEL_SCALE);
    }
    strip.show();
}

// ── Boot sequence ─────────────────────────────────────────────────────────────

void bootDotAnimation()
{
    // Sweep blue dot 0 → end → 0 over 2000 ms
    const unsigned long DUR = 2000UL;
    unsigned long start = millis();
    while (true)
    {
        unsigned long t = millis() - start;
        if (t >= DUR)
            break;
        int sp = (t < DUR / 2)
                     ? map((long)t, 0, DUR / 2, ACTIVE_MIN_SUBPIX, ACTIVE_MAX_SUBPIX)
                     : map((long)t, DUR / 2, DUR, ACTIVE_MAX_SUBPIX, ACTIVE_MIN_SUBPIX);
        showBlue(sp);
    }
    showBlue(ACTIVE_MIN_SUBPIX);
}

// ── Setup & Loop ──────────────────────────────────────────────────────────────
void printSketchNameAndCompileDate()
{
    String the_path = __FILE__;
    int slash_loc = the_path.lastIndexOf('/');
    int backslash_loc = the_path.lastIndexOf('\\');
    int sep_loc = (slash_loc > backslash_loc) ? slash_loc : backslash_loc;
    String the_cpp_name = the_path.substring(sep_loc + 1);
    int dot_loc = the_cpp_name.lastIndexOf('.');
    String the_sketchname = (dot_loc > 0) ? the_cpp_name.substring(0, dot_loc) : the_cpp_name;

    Serial.print("\nArduino is running Sketch: ");
    Serial.println(the_sketchname);
    Serial.print("Compiled on: ");
    Serial.print(__DATE__);
    Serial.print(" at ");
    Serial.print(__TIME__);
    Serial.print("\n");
}

void setup()
{
    Serial.begin(9600);
    printSketchNameAndCompileDate();

    strip.begin();
    strip.show();

    bootDotAnimation();

    knob.write(0);
    prevFrameMs = millis();
}

void loop()
{
    unsigned long now = millis();

    // Encoder
    long raw = knob.read(); // Encoder reads 0-214, approximately.
    // Serial.println(raw);

    if (raw > lastEncPos) {
        targetSubpix += 1;
    } else if (raw < lastEncPos) {
        targetSubpix -= 1;
    } else {
        return;
    }

    Serial.println("Raw input: " + String(raw) + " | Target subpixel: " + String(targetSubpix));

    lastEncPos = raw;
    // long clamped = constrain(raw, 0L, (long)(PIXEL_COUNT - 1));
    // if (clamped != raw)
    //     knob.write(clamped);

    // if (clamped != lastEncPos)
    // {
    //     // Serial.print(F("Encoder: "));
    //     // Serial.println((int)clamped);
    //     lastEncPos = clamped;
    // }


    // int targetSubpix = map((int)clamped,
    //                        0,
    //                        PIXEL_COUNT - 1,
    //                        ACTIVE_MIN_SUBPIX,
    //                        ACTIVE_MAX_SUBPIX);

    // Slew
    unsigned long elapsed = now - prevFrameMs;
    if (elapsed > 0)
    {
        prevFrameMs = now;
        int maxSteps = (int)((MAX_PIX_PER_SEC * SUBPIXEL_SCALE * (float)elapsed) / 1000.0f);
        if (maxSteps > 0)
        {
            int diff = targetSubpix - currentSubpix;
            if (abs(diff) <= maxSteps)
                currentSubpix = targetSubpix;
            else
                currentSubpix += (diff > 0) ? maxSteps : -maxSteps;
        }
        showBlue(currentSubpix);
    }
}
