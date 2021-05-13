#include <LovyanGFX.hpp>
#include <FastLED.h>
static LGFX lcd;
static LGFX_Sprite sprite(&lcd);

static int32_t width = 239;             // 画像サイズ

struct meter_t
{
  int32_t pivot_x;
  int32_t pivot_y;

  float value = 0;
  float angle = 0;
  float add = 0.3;
  float hue = 43.0;

  void advance(void) {
    if (angle == value) return;
    if (angle < value) {
      angle += add;
      if (angle > value)
        angle = value;
      return;
    }
    angle -= add;
    if (angle < value)
      angle = value;
  }
  void drawGauge()
  {
    float sat = 255 * (1.0 - ((angle / 255) * 0.75 + 0.25));
    CHSV hsv(hue, (int)sat, 255);
    CRGB rgb;
    hsv2rgb_rainbow(hsv, rgb);

    lcd.setPivot(pivot_x, pivot_y);
    sprite.setPaletteColor(1, sprite.color888(rgb.r, rgb.g, rgb.b));
    sprite.pushRotated(angle - 127);
    advance();
  }

  void setValue(float per) {
    value = 255 * per;
    if (value > 255) value = 255;
    if (value < 0) value = 0;
    add = abs(angle - value) / 500;
    if (add < 0.1) add = 0.1;
    if (add > 0.5) add = 0.5;
  }
};

struct meter_t meter1, meter2, meter3, meter4;

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.setFont(&fonts::efontJA_16);
  lcd.setTextSize(1, 1);

  // メーター用スプライト
  sprite.setColorDepth(2);
  sprite.createSprite(3, lcd.height() / 8 - 2);
  sprite.setPivot(1, lcd.height() / 4 - 2);
  sprite.drawFastVLine(0, 2, sprite.height() - 4, 1); //左端指定色用
  sprite.drawFastVLine(1, 0, sprite.height()    , 3); //真ん中（白？）
  //右端（無指定で黒？）

  meter1.pivot_x = lcd.width() >> 2;
  meter1.pivot_y = (lcd.height() >> 2) + 2;
  meter1.hue = HUE_ORANGE;

  meter2.pivot_x = (lcd.width() * 3) >> 2;
  meter2.pivot_y = (lcd.height() >> 2) + 2;
  meter2.hue = HUE_PINK;

  meter3.pivot_x = lcd.width() >> 2;
  meter3.pivot_y = (lcd.height() * 3) >> 2;
  meter3.hue = HUE_BLUE;

  meter4.pivot_x = (lcd.width() * 3) >> 2;
  meter4.pivot_y = (lcd.height() * 3) >> 2;
  meter4.hue = HUE_YELLOW;

  lcd.fillScreen(lcd.color565(0, 0, 0));
  lcd.setTextDatum(lgfx::middle_center);

  int guageWidth = lcd.width() >> 2;
  lcd.fillArc(meter1.pivot_x, meter1.pivot_y, guageWidth - 18, guageWidth - 19, 145,  35, TFT_GREEN);
  lcd.fillArc(meter2.pivot_x, meter2.pivot_y, guageWidth - 18, guageWidth - 19, 145,  35, TFT_GREEN);
  lcd.fillArc(meter3.pivot_x, meter3.pivot_y, guageWidth - 18, guageWidth - 19, 145,  35, TFT_GREEN);
  lcd.fillArc(meter4.pivot_x, meter4.pivot_y, guageWidth - 18, guageWidth - 19, 145,  35, TFT_GREEN);

  lcd.setTextColor(TFT_WHITE);
  lcd.setCursor(meter1.pivot_x - 16, meter1.pivot_y - 16);
  lcd.printf("CPU");

  lcd.setCursor(meter2.pivot_x - 16, meter2.pivot_y - 16);
  lcd.printf("MEM");

  lcd.setCursor(meter3.pivot_x - 16, meter3.pivot_y - 16);
  lcd.printf("NET");

  lcd.setCursor(meter4.pivot_x - 16, meter4.pivot_y - 16);
  lcd.printf("HDD");
}

void setValue(int cpu, int mem, int net, const char* netUnit, int hdd) {
  meter1.setValue(cpu < 0 ? 0 : cpu > 100 ? 1 : (float)cpu / 100);
  meter2.setValue(mem < 0 ? 0 : mem > 100 ? 1 : (float)mem / 100);
  meter3.setValue(net < 0 ? 0 : net > 1000 ? 1 : (float)net / 1000);
  meter4.setValue(hdd < 0 ? 0 : hdd > 100 ? 1 : (float)hdd / 100);

  lcd.setTextDatum(lgfx::middle_right);
  lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
  lcd.setCursor(meter1.pivot_x - 12, meter1.pivot_y);
  lcd.printf("%3d%s", cpu, "%");

  lcd.setTextColor(TFT_MAGENTA, TFT_BLACK);
  lcd.setCursor(meter2.pivot_x - 12, meter2.pivot_y);
  lcd.printf("%3d%s", mem, "%");

  lcd.setTextColor(TFT_CYAN, TFT_BLACK);
  lcd.setCursor(meter3.pivot_x - 12, meter3.pivot_y);
  lcd.printf("%3d%s", net, netUnit);

  lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  lcd.setCursor(meter4.pivot_x - 12, meter4.pivot_y);
  lcd.printf("%3d%s", hdd, "%");
}

void handleSerial(void) {
  if (Serial.available() <= 0)
    return;

  String str = Serial.readStringUntil(';');

  int cpu = str.substring(0, 3).toInt();
  int mem = str.substring(3, 6).toInt();
  int net = str.substring(6, 9).toInt();
  String netUnit = str.substring(9, 10);
  int hdd = str.substring(10, 12).toInt();

  setValue(cpu, mem, net, netUnit.c_str(), hdd);
}

void loop(void) {
  handleSerial();
  meter1.drawGauge();
  meter2.drawGauge();
  meter3.drawGauge();
  meter4.drawGauge();
  delay(1);
}
