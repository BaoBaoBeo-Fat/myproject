#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include "soc/soc_caps.h"
#include <stdint.h>

// ============================================================
// RGB LED -- co dinh phan cung, khong doi duoc (silkscreen board that: "RGB@IO27")
// ============================================================
#define PIN_RGB_LED 27
static const uint8_t LED_BUILTIN = SOC_GPIO_PIN_COUNT + PIN_RGB_LED;
#define BUILTIN_LED LED_BUILTIN
#define LED_BUILTIN LED_BUILTIN
#define RGB_BUILTIN LED_BUILTIN
#define RGB_BRIGHTNESS 64
#define HAS_RGB_LED 1
#define LED_ORDER GRB
#define LED_TYPE_IS_RGBW 1
#define LED_COUNT 1
#define LED_TYPE WS2812
#define LED_COLOR_STEP 15
#define RGB_LED 27

// ============================================================
// USB CDC -- debug qua USB noi bo (GPIO13/14), UART0 (11/12) danh cho GT24
// ============================================================
static const uint8_t USB_DM = 13;
static const uint8_t USB_DP = 14;
static const uint8_t TX = 11;
static const uint8_t RX = 12;
#define SERIAL_TX 11
#define SERIAL_RX 12

// LP UART -- pin co dinh silicon tren ESP32-C5, khong doi
static const uint8_t LP_RX = 4;
static const uint8_t LP_TX = 5;

// ============================================================
// I2C -- GPIO4/5 (Grove, BadUSB, GPS dung chung, mux bang phan mem)
// ============================================================
static const uint8_t SDA = 4;
static const uint8_t SCL = 5;
static const uint8_t LP_SDA = 4;
static const uint8_t LP_SCL = 5;
#define WIRE1_PIN_DEFINED
#define SDA1 LP_SDA
#define SCL1 LP_SCL
#define GROVE_SDA 4
#define GROVE_SCL 5
#define BAD_RX 4
#define BAD_TX 5
#define GPS_SERIAL_RX 4
#define GPS_SERIAL_TX 5

// ============================================================
// SPI bus chung -- SCK=6(FSPICLK), MISO=2(FSPIQ), MOSI=7(FSPID)
// Day la IOMUX phan cung goc cua chip (xac nhan qua so do chinh hang
// ESP32-C5-WROOM-1), toc do nhanh nhat co the, khong qua GPIO Matrix
// ============================================================
static const uint8_t SS   = 10;
static const uint8_t MOSI = 7;
static const uint8_t MISO = 2;
static const uint8_t SCK  = 6;
#define SPI_SCK_PIN  6
#define SPI_MOSI_PIN 7
#define SPI_MISO_PIN 2
#define SPI_SS_PIN   9

// ADC1 -- CHI co that tren GPIO1-6 (xac nhan qua soc/esp32c5/include/
// soc/adc_channel.h: ADC1_GPIO1..6_CHANNEL = 0..5). GPIO0 KHONG co ADC.
// SUA LOI CU: truoc day ghi nham "GPIO0=ADC1_CH0" -- that ra ADC1_CH0
// nam o GPIO1 (dang dung cho TOUCH_CS, khong dung lai duoc cho ladder).
static const uint8_t A0 = 1;  // ADC1_CH0
static const uint8_t A1 = 2;  // ADC1_CH1
static const uint8_t A2 = 3;  // ADC1_CH2
static const uint8_t A3 = 4;  // ADC1_CH3
static const uint8_t A4 = 5;  // ADC1_CH4
static const uint8_t A5 = 6;  // ADC1_CH5

// ============================================================
// TFT -- ST7789 2.4" 240x320 (SUA: upstream dung ILI9341, thay bang ST7789)
// TFT_CS=23 / TFT_DC=24: KHOI PHUC theo upstream. VOM + so do chinh hang
// xac nhan GPIO23/24 CO THAT (silkscreen in nham so, khong phai khong co).
// ============================================================
#define HAS_SCREEN 1
#define ROTATION 1
#define MINBRIGHT (uint8_t)1
#define USER_SETUP_LOADED 1

#define ST7789_DRIVER 1
#define TFT_WIDTH  240
#define TFT_HEIGHT 320
#define TFT_INVERSION_ON
#define TFT_RGB_ORDER TFT_BGR

#define TFT_BACKLIGHT_ON 1
#define TFT_BL   25
#define TFT_RST  -1
#define TFT_DC   24   // GPIO24 co that (VOM xac nhan)
#define TFT_CS   23   // GPIO23 co that (VOM xac nhan)
#define TFT_MISO  2
#define TFT_MOSI  7
#define TFT_SCLK  6
#define SMOOTH_FONT 1
#define SPI_FREQUENCY       20000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000

// ============================================================
// Touch XPT2046 -- GPIO1, dua ra NGOAI moi #ifdef driver (upstream
// cu nhot trong #ifdef ILI9341_DRIVER, sua de dung chung voi ST7789)
// TAM TAT de test co lap: IC cam ung da thao khoi board vat ly, nhung
// code dung #ifdef HAS_TOUCH (kiem tra macro CO duoc define hay khong,
// khong quan tam gia tri) -- vi vay phai COMMENT HAN dong define, doi
// "HAS_TOUCH 0" se KHONG tat duoc gi ca. Mo lai khi gan IC that vao.
// ============================================================
#define USE_TFT_eSPI_TOUCH 1
// #define HAS_TOUCH 1   // TAM TAT -- IC da thao, test co lap phantom input
#define TOUCH_CS  1
#define TOUCH_INT -1

// ============================================================
// Nut 5 huong -- Resistor ladder GPIO3 (ADC1_CH2), toan tro 10k
// SUA: truoc gan GPIO0 -- GPIO0 KHONG CO ADC tren C5 (xac nhan qua
// soc/esp32c5/include/soc/adc_channel.h) -> analogRead(0) luon tra 0
// -> ladder luon doc ra "CENTER" gia -> nut OK bi "nhan ao" vinh vien.
// Doi sang GPIO3 (ADC1_CH2, co ADC that). CAN HAN LAI DAY VAT LY: doi
// cho day ladder (0->3) va day IR TXLED (3->0) cho nhau tren board that.
// ============================================================
#define BTN_LADDER_PIN 3
#define HAS_5_BUTTONS 1
#define BTN_ACT LOW
#define DEEPSLEEP_WAKEUP_PIN 3

// ============================================================
// InfraRed thu + phat
// TXLED SUA: truoc GPIO3, doi sang GPIO0 de nhuong GPIO3 (ADC1_CH2)
// cho resistor ladder. GPIO0 AN TOAN vi 2 ly do: (1) KHONG nam trong
// danh sach strapping pin that cua C5 -- theo tai lieu chinh thuc
// Espressif, strapping pin tren C5 la GPIO2,3,7,25,26,27,28 (khac han
// ESP32 classic noi GPIO0 la strap chinh); (2) GPIO0 tren C5 chi co
// chuc nang XTAL_32K_P, khong ADC, nen dung lam digital output cho
// IR LED hoan toan an toan, khong anh huong gi den qua trinh boot.
// CAN HAN LAI DAY VAT LY day IR TX tu GPIO3 sang GPIO0.
// ============================================================
#define RXLED  26
#define TXLED   0
#define LED_ON  HIGH
#define LED_OFF LOW

// ============================================================
// SD Card
// ============================================================
#define SDCARD_CS   10
#define SDCARD_SCK  SPI_SCK_PIN
#define SDCARD_MISO SPI_MISO_PIN
#define SDCARD_MOSI SPI_MOSI_PIN

// ============================================================
// CC1101 Sub-GHz RF
// ============================================================
#define CC1101_GDO0_PIN  8
#define CC1101_SS_PIN    9
#define CC1101_MOSI_PIN  SPI_MOSI_PIN
#define CC1101_SCK_PIN   SPI_SCK_PIN
#define CC1101_MISO_PIN  SPI_MISO_PIN

// ============================================================
// GT24 (nRF24L01+ high power) -- SUA: upstream goc dung CE=8,SS=9
// TRUNG VOI CC1101 (GDO0=8, SS=9) -- xung dot pin thuc su chua ai
// phat hien. Doi sang GPIO11(SS)/GPIO12(CE), repurpose tu UART0
// TX/RX (debug qua USB CDC, khong qua UART0 nua).
// ============================================================
#define NRF24_SS_PIN   11
#define NRF24_CE_PIN   12
#define NRF24_MOSI_PIN SPI_MOSI_PIN
#define NRF24_SCK_PIN  SPI_SCK_PIN
#define NRF24_MISO_PIN SPI_MISO_PIN

// ============================================================
// Ethernet W5500 (du phong, khong dung nhung giu dinh nghia)
// ============================================================
#define W5500_INT_PIN  8
#define W5500_SS_PIN   9
#define W5500_MOSI_PIN SPI_MOSI_PIN
#define W5500_SCK_PIN  SPI_SCK_PIN
#define W5500_MISO_PIN SPI_MISO_PIN

// ============================================================
// SPI bus -- C5 CHI CO 1 SPI (FSPI/SPI2 = bus index 0).
// KHONG dinh nghia USE_HSPI_PORT: no lam TFT_eSPI dung HSPI
// (bus 1 = SPI3, khong ton tai tren C5) -> spi.begin() fail
// -> man hinh trang. TFT va AUX_SPI deu phai dung FSPI.
// ============================================================

#endif /* Pins_Arduino_h */
