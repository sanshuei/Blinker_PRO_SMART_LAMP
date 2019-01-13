#include "SMART_LAMP_control.h"

// #include <WS2812FX.h>

// // Parameter 1 = number of pixels in strip
// // Parameter 2 = Arduino pin number (most are valid)
// // Parameter 3 = pixel type flags, add together as needed:
// //   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
// //   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
// //   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
// //   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
// //   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
// WS2812FX ws2812fx = WS2812FX(BLINKER_WS2812_COUNT, BLINKER_WS2812_PIN, NEO_RGB + NEO_KHZ800);


#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 4

// #define BLINKER_POWER_3V3_PIN 14
// #define BLINKER_POWER_5V_PIN 15

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(BLINKER_WS2812_COUNT, BLINKER_WS2812_PIN, NEO_GRB + NEO_KHZ800);

static bool     change = false;
static bool     lamp_state = true;
static uint8_t  lampType = BLINKER_LAMP_RAINBOW_CYCLE;
static uint32_t lampStep = 0;
static uint32_t lampSpeed = 100;//BLINKER_LAMP_SPEED_DEFUALT;
static uint32_t _lampSpeed = 100;//BLINKER_LAMP_SPEED_DEFUALT;
static uint32_t freshStart = 0;

static uint32_t stream_color[4] = {0xff0000, 0x00ff00, 0x0000ff, 0xffffff};
static uint8_t  stream_num = 0;
static uint8_t  stream_count = 4;
static uint32_t stream_color_standard = 0;

static uint32_t standard_color = 0x88ff0088;

static uint32_t latest_color = 0;
static uint32_t now_color = 0;
static bool     isGraded = true;

static uint32_t latest_brt = 0;
static uint32_t now_brt = 0;
static bool     isBrt = true;
static uint32_t brtStep = 0;
static uint32_t brtTime = 0;

// static callback_with_uint32_arg_t _lampDelay = NULL;

bool needFresh()
{
    // strip.show();

    // if (_lampSpeed < 256) _lampSpeed = 256;

    if (_lampSpeed < 1) _lampSpeed = 1;

    // yield();

    if ((millis() - freshStart) >= _lampSpeed) {
        freshStart = millis();
        return true;
    }
    else {
        return false;
    }
}

// void lampFresh(uint32_t ms)
// {
//     ms = ms * BLINKER_WS2812_COUNT / 256;
    
//     // while(!needFresh());

//     if (_lampDelay) {
//         _lampDelay(ms);
//     }
//     else {
//         delay(ms);
//     }
// }

uint32_t Wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if(WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void colorWipe(uint32_t c, uint8_t wait)
{
    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
    }
    strip.show();
    // lampFresh(wait);
}

void rainbowDisplay()
{
    for(uint8_t i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel((i + lampStep) & 255));
    }
    lampStep = (lampStep + 1) & 0xFF;

    strip.show();
    // lampFresh(lampSpeed);
}

void rainbowCycleDisplay() {
    for(uint8_t i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + lampStep) & 255));
    }
    lampStep = (lampStep + 1) & 0xFF;//(lampStep + getSpeed()/25) & 0xFF;

    strip.show();
    // lampFresh(lampSpeed);
}

// uint32_t streamer(uint32_t c) {
uint32_t streamer() {
    if (!lamp_state) {
        uint8_t r   = (stream_color_standard >> 16 & 0xFF);
        uint8_t g   = (stream_color_standard >>  8 & 0xFF);
        uint8_t b   = (stream_color_standard       & 0xFF);

        for(uint8_t i=0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, r, g, b);
        }

        strip.show();

        return 0;
    }

    if (lampStep == 0) stream_num = (stream_num + 1) % stream_count;

    int lum = lampStep;

    uint16_t start_lum = 64;
    uint16_t end_lum = 448;

    uint32_t c = stream_color[stream_num];

    uint8_t now_r = (c >> 16 & 0xFF);
    uint8_t now_g = (c >>  8 & 0xFF);
    uint8_t now_b = (c       & 0xFF);

    uint8_t next_num = (stream_num + 1) % stream_count;

    if (lum < end_lum) next_num = (stream_num + stream_count - 1) % stream_count;

    uint32_t next_c = stream_color[next_num];

    uint8_t next_r = (next_c >> 16 & 0xFF);
    uint8_t next_g = (next_c >>  8 & 0xFF);
    uint8_t next_b = (next_c       & 0xFF);

    uint8_t r, g, b;

    if (lum < start_lum) {
        r = next_r;
        g = next_g;
        b = next_b;
    }
    else if (lum >= start_lum && lum <= end_lum){
        r = (next_r - (next_r - now_r) * (lum - start_lum) / (end_lum - start_lum));
        g = (next_g - (next_g - now_g) * (lum - start_lum) / (end_lum - start_lum));
        b = (next_b - (next_b - now_b) * (lum - start_lum) / (end_lum - start_lum));
    }
    else {
        r = now_r;
        g = now_g;
        b = now_b;
    }

    uint32_t _delay = lampSpeed * 2 / 5;

    if (lum < start_lum || lum > end_lum) _delay = lampSpeed;// * 8 / 5;

    for(uint8_t i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, r, g, b);
    }

    strip.show();

    lampStep += 2;
    if(lampStep > 512) lampStep = 0;

    return _delay;
}

uint32_t standard()
{
    if (!isGraded) return colorGradient();

    uint8_t lum = (standard_color >> 24 & 0xFF);
    uint8_t r   = (standard_color >> 16 & 0xFF);// * lum / 256;
    uint8_t g   = (standard_color >>  8 & 0xFF);// * lum / 256;
    uint8_t b   = (standard_color       & 0xFF);// * lum / 256;

    // strip.setBrightness(lum);

    for(uint8_t i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, r, g, b);
    }

    strip.show();

    return 0;
}

uint32_t breath()
{
    if (!isGraded) return colorGradient();

    int lum = lampStep;
    if(lum > 255) lum = 511 - lum; // lum = 15 -> 255 -> 15

    uint32_t _delay;
    if(lum == 15) _delay = lampSpeed * 12 / 10;//50; // 970 pause before each breath
    else if(lum <=  25 && lum > 15)  _delay = lampSpeed * 6 / 10;// * 2;//38; // 19
    else if(lum <=  50 && lum > 25)  _delay = lampSpeed * 5 / 10;// * 2;//36; // 18
    else if(lum <=  75 && lum > 50)  _delay = lampSpeed * 4 / 10;// * 2;//28; // 14
    else if(lum <= 100 && lum > 75)  _delay = lampSpeed * 3 / 10;//20; // 10
    else if(lum <= 125 && lum > 100) _delay = lampSpeed * 2 / 10;//14; // 7
    else if(lum <= 150 && lum > 125) _delay = lampSpeed * 1.5 / 10;//11; // 5
    else _delay = lampSpeed * 1 / 10;//10; // 4

    // uint32_t color = SEGMENT.colors[0];
    // uint8_t w = (standard_color >> 24 & 0xFF) * lum / 256;
    uint8_t r = (standard_color >> 16 & 0xFF) * lum / 256;
    uint8_t g = (standard_color >>  8 & 0xFF) * lum / 256;
    uint8_t b = (standard_color       & 0xFF) * lum / 256;
    for(uint16_t i = 0; i <= strip.numPixels(); i++) {
        strip.setPixelColor(i, r, g, b);
    }

    strip.show();

    lampStep += 2;
    if(lampStep > (512 - 50)) lampStep = 50;

    return _delay;
}

uint32_t rainbowStrobe()
{
    uint32_t c;
    uint32_t _delay;
    if (lampStep % 2) {
        // c = Wheel(lampStep);
        c = Wheel(random(0, 255));
        _delay = lampSpeed;//5000;
    }
    else {
        c = 0;
        _delay = lampSpeed;// * 20;//1000;
    }

    if (lampStep == 255) lampStep = 0;
    else lampStep++;
    
    uint8_t r = (c >> 16 & 0xFF);
    uint8_t g = (c >>  8 & 0xFF);
    uint8_t b = (c       & 0xFF);
    
    for(uint16_t i = 0; i <= strip.numPixels(); i++) {
        strip.setPixelColor(i, r, g, b);
    }

    strip.show();

    return _delay;
}

void resetDisplay(uint16_t _time)
{
    uint8_t lum = map(_time, 0, 10000, 255, 0);
    uint8_t r   = 255 - lum;
    uint8_t g   = lum;
    uint8_t b   = lum;

    for(uint8_t i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, r, g, b);
    }

    strip.show();
}

// void attachDelay(callback_with_uint32_arg_t newFunc)
// {
//     _lampDelay = newFunc;
// }

uint32_t colorGradient()
{
    int lum = lampStep;

    uint8_t latest_r = (latest_color >> 16 & 0xFF);
    uint8_t latest_g = (latest_color >>  8 & 0xFF);
    uint8_t latest_b = (latest_color       & 0xFF);

    uint8_t now_r = (now_color >> 16 & 0xFF);
    uint8_t now_g = (now_color >>  8 & 0xFF);
    uint8_t now_b = (now_color       & 0xFF);

    uint8_t r = (latest_r + (now_r - latest_r) * (lum) / 128);
    uint8_t g = (latest_g + (now_g - latest_g) * (lum) / 128);
    uint8_t b = (latest_b + (now_b - latest_b) * (lum) / 128);

    uint32_t _delay = 128;

    for(uint8_t i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, r, g, b);
    }

    strip.show();

    lampStep += 2;
    if(lampStep > 128) 
    {
        isGraded = true;
        lampStep = 256;
    }

    return _delay;
}

void setStandard(uint32_t color)
{
    latest_color = strip.getPixelColor(0);
    now_color = color;
    isGraded = false;
    lampStep = 0;

    standard_color = color;
}

void setStreamer(uint8_t num, uint32_t color)
{
    if (num > BLINKER_LAMP_COLOR_B) return;

    stream_color[num] = color;
    stream_color_standard = color;
}

void setStreamer(uint32_t *color)
{
    memcpy(stream_color, color, sizeof(color));
}

void setBrightness(uint8_t bright)
{
    latest_brt = getBrightness();
    now_brt = bright;
    isBrt = false;
    brtStep = 0;
    brtTime = millis();
    // strip.setBrightness(bright);
}

uint8_t getBrightness()
{
    return strip.getBrightness();
}

uint32_t getPixelColor()
{
    return strip.getPixelColor(0);
}

uint8_t getSpeed()
{
    return 255 - lampSpeed;//map(lampSpeed, BLINKER_LAMP_SPEED_MAX, BLINKER_LAMP_SPEED_MIN, 0, 255);
}

void setSpeed(uint8_t speed)
{
    lampSpeed = 255 - speed;
    // speed = speed * 256;
    // if (speed < BLINKER_LAMP_SPEED_MIN)
    //     speed = BLINKER_LAMP_SPEED_MIN;
    // if (speed > BLINKER_LAMP_SPEED_MAX)
    //     speed = BLINKER_LAMP_SPEED_MAX;
    
    // lampSpeed = map(speed, 0, 255, BLINKER_LAMP_SPEED_MAX, BLINKER_LAMP_SPEED_MIN);
    _lampSpeed = lampSpeed;
}

uint8_t getMode()
{
    return lampType;
}

void setMode(uint8_t _mode)
{
    lampType = _mode;
}

void modeChange()
{
    lampType = (lampType + 1) % BLINKER_LAMP_TYPE_COUNT;
    lampStep = 0;
    lampSpeed = 255;//BLINKER_LAMP_SPEED_DEFUALT;
    _lampSpeed = lampSpeed;

    Serial.println(lampType);
}

void rainbow()
{
    lampType = BLINKER_LAMP_RAINBOW;
}

void rainbowCycle()
{
    lampType = BLINKER_LAMP_RAINBOW_CYCLE;
}

void setLampMode(uint8_t lamp_mode, bool state)
{
    lampType = lamp_mode;

    lamp_state = state;
}

void lumiBreath()
{
    if (!isBrt)
    {
        if (millis() - brtTime > 2)
        {
            brtTime = millis();
            
            uint8_t set_brt = latest_brt + (now_brt - latest_brt) * brtStep / 128;

            brtStep += 2;
            if (brtStep > 128) isBrt = true;

            strip.setBrightness(set_brt);
            strip.show();

            // Serial.print("SET BRT: ");
            // Serial.println(set_brt);
        }
    }
}

void ledInit()
{
    strip.begin();
    strip.show();
    strip.setBrightness(255);

    ledRun();
}

void ledRun()
{
    lumiBreath();

    if (!needFresh()) return;

    switch(lampType) {
        case BLINKER_LAMP_RAINBOW_CYCLE :
            rainbowCycleDisplay();
            break;
        case BLINKER_LAMP_RAINBOW :
            rainbowDisplay();
            break;
        case BLINKER_LAMP_STREAMER :
            _lampSpeed = streamer();// * 256;
            break;
        case BLINKER_LAMP_STANDARD :
            _lampSpeed = standard();
            break;
        case BLINKER_LAMP_BREATH :
            _lampSpeed = breath();// * 256;
            break;
        case BLINKER_LAMP_RAINBOW_STROBE :
            _lampSpeed = rainbowStrobe();// * 256;
            break;
        default :
            break;
    }
}