#include "core/powerSave.h"
#include "core/utils.h"
#include <interface.h>

// ============================================================
// Resistor ladder GPIO3 (ADC1_CH2 -- SUA: truoc GPIO0 khong co ADC
// tren C5, da doi sang GPIO3, xem ly do day du trong pins_arduino.h).
// Toan tro 10k (het tro gia tri khac, ghep noi tiep/song song de tao
// cac muc dien ap khac nhau)
//
// CENTER(Select) : 0 Ohm (noi tat)        -> ~0.00V -> ADC ~0
// DOWN           : 2x10k SONG SONG = 5k   -> ~1.10V -> ADC ~1364
// LEFT           : 1x10k                  -> ~1.65V -> ADC ~2048
// RIGHT          : 2x10k NOI TIEP = 20k   -> ~2.20V -> ADC ~2731
// UP             : 4x10k NOI TIEP = 40k   -> ~2.64V -> ADC ~3277
// NONE (kh. nhan): ho mach, keo len 3.3V  -> ~3.30V -> ADC ~4095
// (pull-up rieng 1x10k tu 3V3 ve GPIO3, 10 tro dung het trong 20 tro)
// ============================================================
#define LADDER_CENTER_MAX   550   // 0 Ohm -- midpoint voi DOWN tu so lieu that
#define LADDER_DOWN_MAX     1370  // ~4.8k -- midpoint voi LEFT tu so lieu that
#define LADDER_LEFT_MAX     1930  // ~9.8k -- midpoint voi RIGHT tu so lieu that
#define LADDER_RIGHT_MAX    2420  // ~19.5k -- midpoint voi UP tu so lieu that
#define LADDER_UP_MAX       2975  // ~38.7k -- midpoint voi NONE tu so lieu that
// > 2975 = NONE (~3307 do thuc te, khong bam gi)
//
// Threshold nay tinh tu so lieu ADC THAT do qua Serial (khong con dung
// so ly thuyet cu): enter=1, down=1093, left=1650, right=2206, up=2640,
// khong bam=3307 (idle on dinh). Xac nhan qua phep tinh R_nut // 11.5k
// (nhanh pull-up song song) khop voi thiet ke goc.

#define LADDER_DEBOUNCE_MS 150

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() {

    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH);
    pinMode(TFT_MOSI, OUTPUT);
    digitalWrite(TFT_MOSI, HIGH);
    pinMode(TFT_SCLK, OUTPUT);

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    // TFT_RST = -1 tren board nay (noi EN), khong co pin rieng -> bo qua pinMode(TFT_RST,...)
    pinMode(TFT_DC, OUTPUT);
    digitalWrite(TFT_DC, HIGH);

    pinMode(NRF24_SS_PIN, OUTPUT);
    pinMode(CC1101_SS_PIN, OUTPUT);
    pinMode(SDCARD_CS, OUTPUT);
    pinMode(W5500_SS_PIN, OUTPUT);
    pinMode(TFT_CS, OUTPUT);

    digitalWrite(NRF24_SS_PIN, HIGH);
    digitalWrite(CC1101_SS_PIN, HIGH);
    digitalWrite(SDCARD_CS, HIGH);
    digitalWrite(W5500_SS_PIN, HIGH);
    digitalWrite(TFT_CS, HIGH);

    // GT24 CE LOW luc boot (standby mode)
    pinMode(NRF24_CE_PIN, OUTPUT);
    digitalWrite(NRF24_CE_PIN, LOW);

    // TOUCH_CS (GPIO1): dat OUTPUT HIGH co dinh O DAY, KHONG dat trong
    // #ifdef HAS_TOUCH o _post_setup_gpio() nhu truoc -- vi _readLadder()
    // can digitalRead(TOUCH_CS) luon doc dung HIGH on dinh de biet bus SPI
    // dang ranh, BAT KE touch co bat hay khong. Neu de trong #ifdef HAS_TOUCH
    // va tat touch de test, GPIO1 se thanh floating pin -> doc ngau nhien
    // -> ladder chap chon khong lien quan gi toi touch that ca.
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);

    // GPIO3: ADC input cho resistor ladder -- khong pullup/pulldown noi,
    // dung full-range attenuation de doc het 0-3.3V
    pinMode(BTN_LADDER_PIN, INPUT);
    analogSetAttenuation(ADC_11db);
}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   second stage gpio setup to make a few functions work
***************************************************************************************/
void _post_setup_gpio() {
#ifdef HAS_TOUCH
    // TOUCH_CS pinMode/digitalWrite da chuyen sang _setup_gpio() (chay
    // luon, khong phu thuoc HAS_TOUCH) vi _readLadder() can no on dinh.

    uint16_t calData[5];
    File caldata = LittleFS.open("/calData", "r");

    if (!caldata) {
        tft.setRotation(ROTATION);
        tft.calibrateTouch(calData, TFT_WHITE, TFT_BLACK, 10);

        caldata = LittleFS.open("/calData", "w");
        if (caldata) {
            caldata.printf(
                "%d\n%d\n%d\n%d\n%d\n", calData[0], calData[1], calData[2], calData[3], calData[4]
            );
            caldata.close();
        }
    } else {
        Serial.print("\ntft Calibration data: ");
        for (int i = 0; i < 5; i++) {
            String line = caldata.readStringUntil('\n');
            calData[i] = line.toInt();
            Serial.printf("%d, ", calData[i]);
        }
        Serial.println();
        caldata.close();
    }
    tft.setTouch(calData);
#endif
}

/***************************************************************************************
** Function name: getBattery()
** location: display.cpp
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() { return 0; }

/***************************************************************************************
** Function name: isCharging()
** Description:   Default implementation that returns false
***************************************************************************************/
bool isCharging() { return false; }

/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) {
    if (brightval == 0) {
        analogWrite(TFT_BL, brightval);
    } else {
        int bl = MINBRIGHT + round(((255 - MINBRIGHT) * brightval / 100));
        analogWrite(TFT_BL, bl);
    }
}

/*********************************************************************
** Function: _readLadder
** Chi doc GPIO3 khi TOUCH_CS dang HIGH (bus SPI ranh, khong nhieu cheo
** tu XPT2046). Oversampling 4 lan de on dinh gia tri.
** Tra ve: 0=NONE, 1=UP, 2=DOWN, 3=LEFT, 4=RIGHT, 5=CENTER/SELECT
**********************************************************************/
static uint8_t _readLadder() {
    if (digitalRead(TOUCH_CS) == LOW) return 0;

    // Tang oversampling len 16 lan (thay vi 4) de doc on dinh hon voi
    // tro nguon cao -- an toan hon analogSetCycles() vi khong chac ham
    // do con ton tai/hoat dong dung tren driver ADC moi (adc_oneshot)
    // cua core 3.3.x dung cho C5. Khong dung analogSetCycles().
    int32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += analogRead(BTN_LADDER_PIN);
        delayMicroseconds(200);
    }
    int adc = sum / 16;

    if (adc <= LADDER_CENTER_MAX) return 5;               // CENTER/SELECT
    if (adc <= LADDER_DOWN_MAX) return 2;                 // DOWN
    if (adc <= LADDER_LEFT_MAX) return 3;                 // LEFT
    if (adc <= LADDER_RIGHT_MAX) return 4;                // RIGHT
    if (adc <= LADDER_UP_MAX) return 1;                   // UP
    return 0;                                             // NONE
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
    static unsigned long tm = 0;
    static unsigned long ladderTm = 0;
    static uint8_t lastLadder = 0;

    if (millis() - tm < 200 && !LongPress) return;

#ifdef HAS_TOUCH
    TouchPoint t;
    checkPowerSaveTime();
    bool _IH_touched = tft.getTouch(&t.x, &t.y);
    if (_IH_touched) {
        NextPress = false;
        PrevPress = false;
        UpPress = false;
        DownPress = false;
        SelPress = false;
        EscPress = false;
        AnyKeyPress = false;
        NextPagePress = false;
        PrevPagePress = false;
        touchPoint.pressed = false;
        _IH_touched = false;
        Serial.printf("\nRAW: Touch Pressed on x=%d, y=%d", t.x, t.y);
        if (bruceConfigPins.rotation == 3) {
            t.y = (tftHeight + 20) - t.y;
            t.x = tftWidth - t.x;
        }
        if (bruceConfigPins.rotation == 0) {
            uint16_t tmp = t.x;
            t.x = map((tftHeight + 20) - t.y, 0, 320, 0, 240);
            t.y = map(tmp, 0, 240, 0, 320);
        }
        if (bruceConfigPins.rotation == 2) {
            uint16_t tmp = t.x;
            t.x = map(t.y, 0, 320, 0, 240);
            t.y = map(tftWidth - tmp, 0, 240, 0, 320);
        }

        Serial.printf("\nROT: Touch Pressed on x=%d, y=%d, rot=%d\n", t.x, t.y, bruceConfigPins.rotation);

        if (!wakeUpScreen()) AnyKeyPress = true;
        else return;

        touchPoint.x = t.x;
        touchPoint.y = t.y;
        touchPoint.pressed = true;
        touchHeatMap(touchPoint);
        tm = millis();
        return; // da xu ly touch -> bo qua ladder trong chu ky nay
    }
#endif

#ifdef HAS_5_BUTTONS
    // Gioi han toc do poll ADC, tranh doc lien tuc qua nhanh
    if (millis() - ladderTm < LADDER_DEBOUNCE_MS) return;
    ladderTm = millis();

    uint8_t ladder = _readLadder();

    // Yeu cau 2 lan doc lien tiep giong nhau moi xac nhan -> chong nhieu
    if (ladder != lastLadder) {
        lastLadder = ladder;
        return;
    }

    if (ladder == 0) {
        UpPress = false;
        DownPress = false;
        PrevPress = false;
        NextPress = false;
        SelPress = false;
        EscPress = false;
        AnyKeyPress = false;
        return;
    }

    if (wakeUpScreen()) {
        tm = millis();
        return;
    }

    AnyKeyPress = true;
    tm = millis();

    UpPress = (ladder == 1);
    DownPress = (ladder == 2);
    PrevPress = (ladder == 3);  // LEFT
    NextPress = (ladder == 4);  // RIGHT
    SelPress = (ladder == 5);   // CENTER

    // Giu lau Center = Esc: tat SelPress de khong vua chon vua thoat
    // cung luc. LongPress la co global duoc he thong set khi nhan giu du lau.
    if (SelPress && LongPress) {
        SelPress = false;
        EscPress = true;
    } else {
        EscPress = false;
    }
#endif
}

/*********************************************************************
** Function: powerOff
** location: mykeyboard.cpp
** Turns off the device (or try to)
**********************************************************************/
void powerOff() {}

/*********************************************************************
** Function: checkReboot
** location: mykeyboard.cpp
** Btn logic to turnoff the device (name is odd btw)
**********************************************************************/
void checkReboot() {}
