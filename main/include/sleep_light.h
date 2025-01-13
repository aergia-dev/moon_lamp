
#include <stdio.h>
#include <stdbool.h>
#include "ble_protocol.h"
#include "common_info.h"

typedef enum
{
  AliceBlue = 0x00F0F8FF,
  Amethyst = 0x009966CC,
  AntiqueWhite = 0x00FAEBD7,
  Aqua = 0x0000FFFF,
  Aquamarine = 0x007FFFD4,
  Azure = 0x00F0FFFF,
  Beige = 0x00F5F5DC,
  Bisque = 0x00FFE4C4,
  Black = 0x00000000,
  BlanchedAlmond = 0x00FFEBCD,
  Blue = 0x000000FF,
  BlueViolet = 0x008A2BE2,
  Brown = 0x00A52A2A,
  BurlyWood = 0x00DEB887,
  CadetBlue = 0x005F9EA0,
  Chartreuse = 0x007FFF00,
  Chocolate = 0x00D2691E,
  Coral = 0x00FF7F50,
  CornflowerBlue = 0x006495ED,
  Cornsilk = 0x00FFF8DC,
  Crimson = 0x00DC143C,
  Cyan = 0x0000FFFF,
  DarkSalmon = 0x00E9967A,
  DarkSeaGreen = 0x008FBC8F,
  DarkSlateBlue = 0x00483D8B,
  DarkSlateGray = 0x002F4F4F,
  DarkSlateGrey = 0x002F4F4F,
  DarkTurquoise = 0x0000CED1,
  DarkViolet = 0x009400D3,
  DeepPink = 0x00FF1493,
  DeepSkyBlue = 0x0000BFFF,
  DimGray = 0x00696969,
  DimGrey = 0x00696969,
  DodgerBlue = 0x001E90FF,
  FireBrick = 0x00B22222,
  FloralWhite = 0x00FFFAF0,
  ForestGreen = 0x00228B22,
  Fuchsia = 0x00FF00FF,
  Gainsboro = 0x00DCDCDC,
  GhostWhite = 0x00F8F8FF,
  Gold = 0x00FFD700,
  Goldenrod = 0x00DAA520,
  Gray = 0x00808080,
  Grey = 0x00808080,
  Green = 0x00008000,
  GreenYellow = 0x00ADFF2F,
  Honeydew = 0x00F0FFF0,
  HotPink = 0x00FF69B4,
  IndianRed = 0x00CD5C5C,
  Indigo = 0x004B0082,
  Ivory = 0x00FFFFF0,
  Khaki = 0x00F0E68C,
  Lavender = 0x00E6E6FA,
  LavenderBlush = 0x00FFF0F5,
  LawnGreen = 0x007CFC00,
  LemonChiffon = 0x00FFFACD,
  LightBlue = 0x00ADD8E6,
  LightCoral = 0x00F08080,
  LightCyan = 0x00E0FFFF,
  LightGoldenrodYellow = 0x00FAFAD2,
  LightGreen = 0x0090EE90,
  LightGrey = 0x00D3D3D3,
  LightPink = 0x00FFB6C1,
  LightSalmon = 0x00FFA07A,
  LightSeaGreen = 0x0020B2AA,
  LightSkyBlue = 0x0087CEFA,
  LightSlateGray = 0x00778899,
  LightSlateGrey = 0x00778899,
  LightSteelBlue = 0x00B0C4DE,
  LightYellow = 0x00FFFFE0,
  Lime = 0x0000FF00,
  LimeGreen = 0x0032CD32,
  Linen = 0x00FAF0E6,
  Magenta = 0x00FF00FF,
  Maroon = 0x00800000,
  MediumAquamarine = 0x0066CDAA,
  MediumBlue = 0x000000CD,
  MediumOrchid = 0x00BA55D3,
  MediumPurple = 0x009370DB,
  MediumSeaGreen = 0x003CB371,
  MediumSlateBlue = 0x007B68EE,
  MediumSpringGreen = 0x0000FA9A,
  MediumTurquoise = 0x0048D1CC,
  MediumVioletRed = 0x00C71585,
  MidnightBlue = 0x00191970,
  MintCream = 0x00F5FFFA,
  MistyRose = 0x00FFE4E1,
  Moccasin = 0x00FFE4B5,
  NavajoWhite = 0x00FFDEAD,
  Navy = 0x00000080,
  OldLace = 0x00FDF5E6,
  Olive = 0x00808000,
  OliveDrab = 0x006B8E23,
  Orange = 0x00FFA500,
  OrangeRed = 0x00FF4500,
  Orchid = 0x00DA70D6,
  PaleGoldenrod = 0x00EEE8AA,
  PaleGreen = 0x0098FB98,
  PaleTurquoise = 0x00AFEEEE,
  PaleVioletRed = 0x00DB7093,
  PapayaWhip = 0x00FFEFD5,
  PeachPuff = 0x00FFDAB9,
  Peru = 0x00CD853F,
  Pink = 0x00FFC0CB,
  Plaid = 0x00CC5533,
  Plum = 0x00DDA0DD,
  PowderBlue = 0x00B0E0E6,
  Purple = 0x00800080,
  Red = 0x00FF0000,
  RosyBrown = 0x00BC8F8F,
  RoyalBlue = 0x004169E1,
  SaddleBrown = 0x008B4513,
  Salmon = 0x00FA8072,
  SandyBrown = 0x00F4A460,
  SeaGreen = 0x002E8B57,
  Seashell = 0x00FFF5EE,
  Sienna = 0x00A0522D,
  Silver = 0x00C0C0C0,
  SkyBlue = 0x0087CEEB,
  SlateBlue = 0x006A5ACD,
  SlateGray = 0x00708090,
  SlateGrey = 0x00708090,
  Snow = 0x00FFFAFA,
  SpringGreen = 0x0000FF7F,
  SteelBlue = 0x004682B4,
  Tan = 0x00D2B48C,
  Teal = 0x00008080,
  Thistle = 0x00D8BFD8,
  Tomato = 0x00FF6347,
  Turquoise = 0x0040E0D0,
  Violet = 0x00EE82EE,
  Wheat = 0x00F5DEB3,
  White = 0x00FFFFFF,
  WhiteSmoke = 0x00F5F5F5,
  Yellow = 0x00FFFF00,
  YellowGreen = 0x009ACD32,
  FairyLight = 0x00FFE42D,
  FairyLightNCC = 0x00FF9D2A

} ColorCode;

typedef struct
{
  union
  {
    struct
    {
      uint8_t blue;
      uint8_t green;
      uint8_t red;
      uint8_t alpha;
    } argb;
    uint32_t code;
  };
} ARGB;

typedef struct
{
  float blue;
  float green;
  float red;
  float alpha;
} ARGB_float;

typedef enum
{
  DIMMING = -1,
  BRIGHTER = 1,
} Brightness_direction;

void light_init();
void light_on();
void light_off();
void light_on_dimming();
void light_off_dimming();
void toggle_light();
// void darker_light();
// void brighter_light();
// void light_chage_color(ARGB color, int cnt);
// void light_change_rgb(uint8_t r, uint8_t g, uint8_t b);
// void save_color_nvs(uint8_t r, uint8_t g, uint8_t b);
// void get_current_color(uint8_t *color);
// bool get_light_on_off();
// ARGB fromRGB(uint8_t r, uint8_t g, uint8_t b);

bool ble_cont_light_write(led_status_t *status);
bool ble_cont_light(led_status_t *status);
void set_light_state(bool is_on);
bool get_light_state(void);

uint8_t get_brightness(void);
uint32_t get_saved_color_uint32(void);
void change_color_uint32(uint32_t c);
void ble_change_color(led_status_t *status);