/*
 * Museum exhibit – NeoPixel strip controller
 * Controllino Mini (ATmega328P @ 5 V)
 *
 * Encoder:     CONTROLLINO_IN0 (INT0, pin 2), CONTROLLINO_IN1 (INT1, pin 3)
 * NeoPixel:    CONTROLLINO_D6 PIN HEADER only – leave screw terminal unwired
 * Night relay: CONTROLLINO_R0  HIGH 22:00–07:59, LOW during open hours
 *
 * Boot sequence:
 *   1. Print current RTC time over Serial
 *   2. Relay: ON→OFF→ON→OFF, 500 ms each
 *   3. NeoPixel: blue dot sweeps 0→end→0 over 2000 ms
 *
 * ── Setting the RTC ─────────────────────────────────────────────────────────
 *   1. Uncomment #define SET_RTC_TO_COMPILE_TIME below
 *   2. Upload  (RTC is set to compile time)
 *   3. Comment it back out, upload again
 * ────────────────────────────────────────────────────────────────────────────
 */

// #define SET_RTC_TO_COMPILE_TIME

#include <SPI.h>
#include <Controllino.h>
#include <Encoder.h>
#include <Adafruit_NeoPixel.h>

// ── Configuration ────────────────────────────────────────────────────────────

const int PIXEL_COUNT = 215;
const int SUBPIXEL_SCALE = 16;
const float MAX_PIX_PER_SEC = 20.0f;

const String BITBUCKET_URL = "TBD";

// Sub-pixels moved per quadrature count. 16 = 1 pixel per count.
// If the dot moves too fast, increase this value; too slow, decrease it.
// A 128-step/rev encoder in full quadrature gives 512 counts/rev.
const int SUBPIX_PER_COUNT = SUBPIXEL_SCALE;

// Blue-dot palette
const uint8_t ACTIVE_R = 0, ACTIVE_G = 0, ACTIVE_B = 255;
const uint8_t BG_R = 0, BG_G = 3, BG_B = 0;

// Night window
const uint8_t NIGHT_ON_HOUR = 22;
const uint8_t NIGHT_OFF_HOUR = 8;

// ── Hardware ─────────────────────────────────────────────────────────────────

#define NEOPIXEL_PIN CONTROLLINO_D6
#define RELAY_NIGHT CONTROLLINO_D0

Encoder knob(CONTROLLINO_IN0, CONTROLLINO_IN1);
Adafruit_NeoPixel strip(PIXEL_COUNT * 2, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ── State ─────────────────────────────────────────────────────────────────────

const int MAX_SUBPIX = (PIXEL_COUNT - 1) * SUBPIXEL_SCALE;

int currentSubpix = 0;
long lastEncPos = 0;
bool isNight = false;
bool nightRendered = false;
unsigned long prevFrameMs = 0;
unsigned long prevRtcCheckMs = 0;

// ── RTC ──────────────────────────────────────────────────────────────────────

void updateNightState()
{
    uint8_t h = (uint8_t)Controllino_GetHour();
    isNight = (h >= NIGHT_ON_HOUR || h < NIGHT_OFF_HOUR);
}

void printCurrentTime()
{
    Serial.print(F("Boot time: "));
    Serial.print(Controllino_GetDay());
    Serial.print(F("/"));
    Serial.print(Controllino_GetMonth());
    Serial.print(F("/20"));
    Serial.print(Controllino_GetYear());
    Serial.print(F("  "));
    Serial.print(Controllino_GetHour());
    Serial.print(F(":"));
    int m = Controllino_GetMinute();
    if (m < 10)
        Serial.print(F("0"));
    Serial.print(m);
    Serial.print(F(":"));
    int s = Controllino_GetSecond();
    if (s < 10)
        Serial.print(F("0"));
    Serial.println(s);
}

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

void relayBootSequence()
{
    for (int i = 0; i < 2; i++)
    {
        digitalWrite(RELAY_NIGHT, HIGH);
        delay(500);
        digitalWrite(RELAY_NIGHT, LOW);
        delay(500);
    }
}

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
                     ? map((long)t, 0, DUR / 2, 0, MAX_SUBPIX)
                     : map((long)t, DUR / 2, DUR, MAX_SUBPIX, 0);
        showBlue(sp);
    }
    showBlue(0);
}

// ── Setup & Loop ──────────────────────────────────────────────────────────────
void printSketchNameAndCompileDate()
{
    String the_path = __FILE__;
    int slash_loc = the_path.lastIndexOf('/');
    String the_cpp_name = the_path.substring(slash_loc + 1);
    int dot_loc = the_cpp_name.lastIndexOf('.');
    String the_sketchname = the_cpp_name.substring(0, dot_loc);

    Serial.print("\nArduino is running Sketch: ");
    Serial.println(the_sketchname);
    Serial.print("Compiled on: ");
    Serial.print(__DATE__);
    Serial.print(" at ");
    Serial.print(__TIME__);
    Serial.print(" Bitbucket URL with hash: ");
    Serial.print(BITBUCKET_URL);
    Serial.print("\n");
}

void setup()
{
    Serial.begin(9600);
    printSketchNameAndCompileDate();
    Controllino_RTC_init();

#ifdef SET_RTC_TO_COMPILE_TIME
    Controllino_SetTimeDateStrings(__DATE__, __TIME__);
#endif

    printCurrentTime();

    pinMode(RELAY_NIGHT, OUTPUT);
    digitalWrite(RELAY_NIGHT, LOW);

    strip.begin();
    strip.show();

    relayBootSequence();
    bootDotAnimation();

    updateNightState();
    digitalWrite(RELAY_NIGHT, isNight ? HIGH : LOW);
    if (isNight)
    {
        showAllWhite();
        nightRendered = true;
    }

    knob.write(0);
    prevFrameMs = millis();
    prevRtcCheckMs = millis();
}

void loop()
{
    unsigned long now = millis();

    // RTC check once per second
    if (now - prevRtcCheckMs >= 1000UL)
    {
        prevRtcCheckMs = now;
        updateNightState();
        digitalWrite(RELAY_NIGHT, isNight ? HIGH : LOW);
    }

    // Night mode
    if (isNight)
    {
        if (!nightRendered)
        {
            showAllWhite();
            nightRendered = true;
        }
        prevFrameMs = now;
        return;
    }
    nightRendered = false;

    // Encoder
    long raw = knob.read();
    long clamped = constrain(raw, 0L, (long)(PIXEL_COUNT - 1));
    if (clamped != raw)
        knob.write(clamped);

    if (clamped != lastEncPos)
    {
        Serial.print(F("Encoder: "));
        Serial.println((int)clamped);
        lastEncPos = clamped;
    }

    int targetSubpix = (int)clamped * SUBPIX_PER_COUNT;

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
