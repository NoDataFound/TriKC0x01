/*
* ___________      .__ ____  __._________        _______  ____    __________                 ________.__                      
* \__    ___/______|__|    |/ _|\_   ___ \___  __\   _  \/_   |   \______   \__  _  ______  /  _____/|  |   _______  __ ____  
*   |    |  \_  __ \  |      <  /    \  \/\  \/  /  /_\  \|   |    |     ___/\ \/ \/ /    \/   \  ___|  |  /  _ \  \/ // __ \ 
*   |    |   |  | \/  |    |  \ \     \____>    <\  \_/   \   |    |    |     \     /   |  \    \_\  \  |_(  <_> )   /\  ___/ 
*   |____|   |__|  |__|____|__ \ \______  /__/\_ \\_____  /___|____|____|      \/\_/|___|  /\______  /____/\____/ \_/  \___  >
                              \/        \/      \/      \/   /_____/                     \/        \/                      \/ 
 */


#include <Adafruit_NeoPixel.h>
#include<FastLED.h>
#define PIN 6
#define LED_PIN     6
#define BRIGHTNESS  96
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
/*
* code to transmit pwnglove bend sensor input to controller input
*/

// Analog Pins
#define pinBend 3
#define pinAccelX 0
#define pinAccelY 1
#define pinAccelZ 2

// Digital Pins
#define pinDPadUp 2
#define pinDPadDown 3
#define pinDPadLeft 4
#define pinDPadRight 5
#define pinB 6
#define pinA 7
#define pinStart 8
#define pinSelect 9
#define pinMuxCtl0 12
#define pinMuxCtl1 13
// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
// Store Bend/Accel values
int bendT = 0;
int bendI = 0;
int bendM = 0;
int bendR = 0;
int accelX = 0;
int accelY = 0;
int accelZ = 0;

// Which finger we're reading
int finger = 0;

// Current state of digital buttons
unsigned int buttons = 0;

boolean connected = false;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(51, 6, NEO_GRB + NEO_KHZ800);
const uint8_t kMatrixWidth  = 16;
const uint8_t kMatrixHeight = 16;
const bool    kMatrixSerpentineLayout = true;
// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
#define MAX_DIMENSION ((kMatrixWidth>kMatrixHeight) ? kMatrixWidth : kMatrixHeight)
CRGB leds[kMatrixWidth * kMatrixHeight];
static uint16_t x;
static uint16_t y;
static uint16_t z;
uint16_t speed = 20;
uint16_t scale = 30;
uint8_t noise[MAX_DIMENSION][MAX_DIMENSION];

CRGBPalette16 currentPalette( PartyColors_p );
uint8_t       colorLoop = 1;


void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  delay(3000);
  LEDS.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);
  x = random16();
  y = random16();
  z = random16();
  Serial.begin(9600);
  pinMode(pinDPadUp, INPUT);
  pinMode(pinDPadDown, INPUT);
  pinMode(pinDPadLeft, INPUT);
  pinMode(pinDPadRight, INPUT);
  pinMode(pinB, INPUT);
  pinMode(pinA, INPUT);
  pinMode(pinStart, INPUT);
  pinMode(pinSelect, INPUT);
  pinMode(pinMuxCtl0, OUTPUT);
  pinMode(pinMuxCtl1, OUTPUT);
  
  // Set all digital input pins high (enable pullup resistor)
  // All digital pins will be LOW-on
  digitalWrite(pinDPadUp, HIGH);
  digitalWrite(pinDPadDown, HIGH);
  digitalWrite(pinDPadLeft, HIGH);
  digitalWrite(pinDPadRight, HIGH);
  digitalWrite(pinB, HIGH);
  digitalWrite(pinA, HIGH);
  digitalWrite(pinStart, HIGH);
  digitalWrite(pinSelect, HIGH);
  
  analogReference(DEFAULT);
}
void fillnoise8() {
  uint8_t dataSmoothing = 0;
  if( speed < 50) {
    dataSmoothing = 200 - (speed * 4);
  }
  
  for(int i = 0; i < MAX_DIMENSION; i++) {
    int ioffset = scale * i;
    for(int j = 0; j < MAX_DIMENSION; j++) {
      int joffset = scale * j;
      
      uint8_t data = inoise8(x + ioffset,y + joffset,z);

  
      data = qsub8(data,16);
      data = qadd8(data,scale8(data,39));

      if( dataSmoothing ) {
        uint8_t olddata = noise[i][j];
        uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( data, 256 - dataSmoothing);
        data = newdata;
      }
      
      noise[i][j] = data;
    }
  }
  
  z += speed;
  

  x += speed / 8;
  y -= speed / 16;
}

void mapNoiseToLEDsUsingPalette()
{
  static uint8_t ihue=0;
  
  for(int i = 0; i < kMatrixWidth; i++) {
    for(int j = 0; j < kMatrixHeight; j++) {

      uint8_t index = noise[j][i];
      uint8_t bri =   noise[i][j];


      if( colorLoop) { 
        index += ihue;
      }

    
      if( bri > 127 ) {
        bri = 255;
      } else {
        bri = dim8_raw( bri * 2);
      }

      CRGB color = ColorFromPalette( currentPalette, index, bri);
      leds[XY(i,j)] = color;
    }
  }
  
  ihue+=1;
}

void loop() {
 
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 0, 255), 50); // Blue
  // Send a theater pixel chase in...
  theaterChase(strip.Color(127, 127, 127), 50); // White
  theaterChase(strip.Color(127,   0,   0), 50); // Red
  theaterChase(strip.Color(  0,   0, 127), 50); // Blue
  ChangePaletteAndSettingsPeriodically();
    // generate noise data
  fillnoise8();
  

  mapNoiseToLEDsUsingPalette();

  LEDS.show();
{
  while(Serial.available() > 0)
  {
    byte cmd = Serial.read();
    switch(cmd)
    {
      case 'A': // A
        connected = true;
        break;
      case 'D': // D
        connected = false;
        break;
     }
   }
 }
  if(!connected) return;
  
  // Select and read finger
  switch(finger)
  {
    case 0:
      digitalWrite(pinMuxCtl0, LOW);
      digitalWrite(pinMuxCtl1, LOW);
      delay(10);
      bendT = analogRead(pinBend);
      break;
    case 1:
      digitalWrite(pinMuxCtl0, HIGH);
      digitalWrite(pinMuxCtl1, LOW);
      delay(10);
      bendI = analogRead(pinBend);
      break;
    case 2:
      digitalWrite(pinMuxCtl0, LOW);
      digitalWrite(pinMuxCtl1, HIGH);
      delay(10);
      bendM = analogRead(pinBend);
      break;
    case 3:
      digitalWrite(pinMuxCtl0, HIGH);
      digitalWrite(pinMuxCtl1, HIGH);
      delay(10);
      bendR = analogRead(pinBend);
      break;
  }
  
  // increment finger to read
  finger = (finger + 1) % 4;
  
  // read accelerometer
  accelX = analogRead(pinAccelX);
  accelY = analogRead(pinAccelY);
  accelZ = analogRead(pinAccelZ);
  
  // read digital button state
  buttons = readButtons();
  
  // write the current state of all sensors to the serial line
  writeState();
  delay(10);
}

/*
* Returns an integer bitmask of the current digital button state 
* Konami Code Order -- U-D-L-R-B-A-Start-Select
*/
unsigned int readButtons()
{
  // Negate all readings, as pins are low-enable
  return (!digitalRead(pinDPadUp) << 0) |
    (!digitalRead(pinDPadDown) << 1) | 
    (!digitalRead(pinDPadLeft) << 2) | 
    (!digitalRead(pinDPadRight) << 3) | 
    (!digitalRead(pinB) << 4) | 
    (!digitalRead(pinA) << 5) |
    (!digitalRead(pinStart) << 6) |
    (!digitalRead(pinSelect) << 7);
}


void writeState()
{
  // Print output -- 
  Serial.print(buttons);
  Serial.print("\t");
  Serial.print(bendT);
  Serial.print("\t");
  Serial.print(bendI);
  Serial.print("\t");
  Serial.print(bendM);
  Serial.print("\t");
  Serial.print(bendR);
  Serial.print("\t");
  Serial.print(accelX);
  Serial.print("\t");
  Serial.print(accelY);
  Serial.print("\t");
  Serial.println(accelZ);
}

#define HOLD_PALETTES_X_TIMES_AS_LONG 1
void ChangePaletteAndSettingsPeriodically()
{
  uint8_t secondHand = ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
  static uint8_t lastSecond = 99;
  
  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    if( secondHand ==  0)  { currentPalette = RainbowColors_p;         speed = 20; scale = 30; colorLoop = 1; }
    if( secondHand ==  5)  { SetupPurpleAndGreenPalette();             speed = 10; scale = 50; colorLoop = 1; }
    if( secondHand == 10)  { SetupBlackAndWhiteStripedPalette();       speed = 20; scale = 30; colorLoop = 1; }
    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  8; scale =120; colorLoop = 0; }
    if( secondHand == 20)  { currentPalette = CloudColors_p;           speed =  4; scale = 30; colorLoop = 0; }
    if( secondHand == 25)  { currentPalette = LavaColors_p;            speed =  8; scale = 50; colorLoop = 0; }
    if( secondHand == 30)  { currentPalette = OceanColors_p;           speed = 20; scale = 90; colorLoop = 0; }
    if( secondHand == 35)  { currentPalette = PartyColors_p;           speed = 20; scale = 30; colorLoop = 1; }
    if( secondHand == 40)  { SetupRandomPalette();                     speed = 20; scale = 20; colorLoop = 1; }
    if( secondHand == 45)  { SetupRandomPalette();                     speed = 50; scale = 50; colorLoop = 1; }
    if( secondHand == 50)  { SetupRandomPalette();                     speed = 90; scale = 90; colorLoop = 1; }
    if( secondHand == 55)  { currentPalette = RainbowStripeColors_p;   speed = 30; scale = 20; colorLoop = 1; }
  }
}


void SetupRandomPalette()
{
  currentPalette = CRGBPalette16( 
                      CHSV( random8(), 255, 32), 
                      CHSV( random8(), 255, 255), 
                      CHSV( random8(), 128, 255), 
                      CHSV( random8(), 255, 255)); 
}

void SetupBlackAndWhiteStripedPalette()
{
  // 'black out' all 16 palette entries...
  fill_solid( currentPalette, 16, CRGB::Black);
  // and set every fourth one to white.
  currentPalette[0] = CRGB::White;
  currentPalette[4] = CRGB::White;
  currentPalette[8] = CRGB::White;
  currentPalette[12] = CRGB::White;

}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
  CRGB purple = CHSV( HUE_PURPLE, 255, 255);
  CRGB green  = CHSV( HUE_GREEN, 255, 255);
  CRGB black  = CRGB::Black;
  
  currentPalette = CRGBPalette16( 
    green,  green,  black,  black,
    purple, purple, black,  black,
    green,  green,  black,  black,
    purple, purple, black,  black );
}


//
// Mark's xy coordinate mapping code.  See the XYMatrix for more information on it.
//
uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  if( kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }
  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * kMatrixWidth) + x;
    }
  }
  return i;
}
// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
