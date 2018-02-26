#include <stdarg.h>

//-----------------------------------------------------------------------------------
//전역 변수

enum DIGITAL_PINS { TWO = 2, THREE = 3, FOUR = 4, FIVE = 5, SIX = 6, SEVEN = 7, EIGHT = 8, NINE = 9, TEN = 10, ELEVEN = 11, TWELVE = 12, THIRTEEN = 13 }; 
//enum PWM_DPINS { FADE_THREE = 3, FADE_FIVE = 5, FADE_SIX = 6, FADE_NINE = 9, FADE_TEN = 10, FADE_ELEVEN = 11 };

int PWM_DPINS[6] = { DIGITAL_PINS::THREE, DIGITAL_PINS::FIVE, DIGITAL_PINS::SIX, DIGITAL_PINS::NINE, DIGITAL_PINS::TEN, DIGITAL_PINS::ELEVEN };

enum virtualPin { one = 0, two = 1, three = 2, four = 3, five = 4, six = 5 };


const int MAX_DIGITAL_VIRTUAL = 12 + 2; //0,1 번째 인덱스는 사용하지 않음.
boolean g_FadeFlag[MAX_DIGITAL_VIRTUAL] = { false, };
boolean g_LedLightFlag[MAX_DIGITAL_VIRTUAL] = { false, };

//---------------------------------------------------------

//초기화(아두이노 기본 함수)
void setup();

//아두이노 제공 기본 함수
void serialEvent();

//루프(아두이노 기본 함수)
void loop();

//----------------------------------------------------------------------------
class StringManager
{
private:
  static unsigned char GetAsciiToNumber(unsigned char apabetNum)
  {
    //if (!isxdigit(apabetNum)) return 0;
    enum alpaNumLower { a = 10, b, c, d, e, f };
    enum alpaNumUpper { A = 10, B, C, D, E, F };
    switch (apabetNum) {
    case 'A': return alpaNumUpper::A; break;
    case 'B': return alpaNumUpper::B; break;
    case 'C': return alpaNumUpper::C; break;
    case 'D': return alpaNumUpper::D; break;
    case 'E': return alpaNumUpper::E; break;
    case 'F': return alpaNumUpper::F; break;
    case 'a': return alpaNumLower::a; break;
    case 'b': return alpaNumLower::b; break;
    case 'c': return alpaNumLower::c; break;
    case 'd': return alpaNumLower::d; break;
    case 'e': return alpaNumLower::e; break;
    case 'f': return alpaNumLower::f; break;
    default: return (apabetNum - '0'); break;
    }
  }
public:
  static void printHex(unsigned char* msg) {
    Serial.write((char*)msg, HEX);
  }
  static void printHex(char* msg) {
    Serial.write((char*)msg, HEX);
  }


  static void change(String * dest, const String * src)
  {
    int slen = src->length() - 4;

    for (int i = 0; i < slen; i++)
    {
      Serial.print(F("chage ==\n"));
      Serial.println(src->charAt(i));
      *dest += src->charAt(i);
    }
    *dest += '\0';
  }

  /*
  7071500A03020407CS 프레임 구조

  7071은 header => 2byte
  50은 명령어(50 명령어는 LED ON 명령어) => 1byte
  0A는 프레임의 총 길이
  03은 총 몇개의 핀을 이용할 것인지
  02핀 사용 => 1byte
  04핀 사용 => 1byte
  07핀 사용 => 1byte
  CS(체크섬) => 2byte
  */


  //"7071500A03020407CS" => 이 문자를 반으로 줄여서 2개의 hex문자를 1개의 hex로 변환하는 함수
  static void change_twoChar_oneHex(unsigned char * dest, unsigned char * src) {

    Serial.println(F("change_twoChar_oneHex start"));
    int slen = strlen((char*)src) / sizeof(unsigned char);
    if (slen <= 0) return;

    int maxSize = (slen % 2 == 0) ? slen / 2 : slen / 2 + 1;  // 홀수 일때 남는 데이터가 한개 임으로
    Serial.print(F("maxSize  "));
    Serial.println(maxSize);

    int lastindex = 0;
    for (int i = 0, count = 0; i < maxSize; i++, count += 2) {
      if (i == (maxSize - 1) && (slen % 2)) {
        dest[i] = GetAsciiToNumber(src[count]);
        Serial.print(F("dest["));
        Serial.print(i);
        Serial.print(F("] = "));
        Serial.print(dest[i], HEX);
        break;
      }
      dest[i] = ((GetAsciiToNumber(src[count]) << 4) + GetAsciiToNumber(src[count + 1]));

      Serial.print(F("dest["));
      Serial.print(i);
      Serial.print(F("] = "));
      Serial.print(dest[i], HEX);
      Serial.print(F("\n"));
    }
    Serial.println();
    for (int i = 0; i < maxSize; i++) {
      Serial.print(dest[i], HEX);
      Serial.print(F(" "));
      //if ((i + 1) % 8 == 0) Serial.println();
      // if (i == maxSize - 1) Serial.println();
    }
    Serial.println();
  }

  //F1 F2 50 과 f1 f2 50과 체크섬이 같다.0x로 시작하는 hex는 F와 f를 구별하지 않는다.
  //https://forum.arduino.cc/index.php?topic=347504.0
  static byte checksum(unsigned char arr[], int size)
  {
    byte cs = 0;

    Serial.println(F("\nCHECK MAX SIZE ="));
    Serial.println(size);
    for (int i = 0; i < size; i++)
    {
      cs += arr[i];
      Serial.print(arr[i], HEX);
    }

    return cs;
  }
};

class Command
{
public:
  //새로운 명령어 나오면 추가해야 되는 곳
  enum formal { HEADER1 = 0x70, HEADER2 = 0x71 };
  enum led { ON = 0x50, OFF = 0x51, FADE_START = 0x60, FADE_END = 0x61 };
  enum frame { HEAD1 = 0, HEAD2 = 1, CMD = 2, FRAME_LEN = 3, DATA_LEN = 4, PINNUM_ONE = 5, PINNUM_TWO = 6, PINNUM_THREE = 7, PINNUM_FOUR = 8, PINNUM_FIVE = 9, PINNUM_SIX = 10 }; //현재 LEN은 사용되지 않음.
};
class CmdManager
{
public:
  static bool CheckHeader(unsigned char *serial) {
    if (serial[Command::frame::HEAD1] == Command::formal::HEADER1 && serial[Command::frame::HEAD2] == Command::formal::HEADER2)
    {
      return true;
    }
    return false;
  }


  //새로운 명령어 나오면 추가해야 되는 곳
  unsigned char GetCommand(unsigned char *serialCmd) {
    if (serialCmd[Command::frame::HEAD1] != Command::formal::HEADER1 || serialCmd[Command::frame::HEAD2] != Command::formal::HEADER2) return 0;

    switch (serialCmd[Command::frame::CMD]) {
    case Command::led::ON:
      return Command::led::ON;
      break;
    case Command::led::OFF:
      return Command::led::OFF;
      break;
    case Command::led::FADE_START:
      return Command::led::FADE_START;
      break;
    case Command::led::FADE_END:
      return Command::led::FADE_END;
      break;
    }

    return 0;
  }
  /*
  unsigned char GetPin(unsigned char * serialCmd) {
  return serialCmd[Command::frame::PINNUM];
  }
  */

  int GetFrameLen(unsigned char* serialCmd) {
    return serialCmd[Command::frame::FRAME_LEN];
  }
  unsigned char GetDataLen(unsigned char* serialCmd) {
    return serialCmd[Command::frame::DATA_LEN];
  }
  int * GetPins(unsigned char* serialCmd) {

    int pinCounts = GetDataLen(serialCmd);

    Serial.print(F("pinCounts = "));
    Serial.println(pinCounts);

    int pins[10];

    for (int i = 0; i < pinCounts; i++) {
      pins[i] = serialCmd[(Command::frame::PINNUM_ONE) + i];

      Serial.print(F("pins["));
      Serial.print(i);
      Serial.print(F("] = "));
      Serial.println(pins[i]);
    }
    pins[pinCounts] = '\0';
    return pins;
  }
};

//시리얼 데이터 수집 클래스
class SerialCollector
{
private:
  //static long interval;
  //static long perviousMillis;

  static String collection;
  static int count;
  enum check { HEADER1, HEADER2, COMMAND, };
  const int maxCnt = 40;
  const int headerSize = 8; //4 * 2 (현재는 가공 되기전 2개의 문자가 하나의 hex이다)
public:
  void start() {
    /*
    //interval밀리 초 마다 데이터를 삭제한다.
    unsigned long currentMillis = millis();
    if (currentMillis - perviousMillis >= interval) {
      perviousMillis = currentMillis;
      collection = "";
      count = 0;
    }
    */
    while (Serial.available()) {
      char userCmd = (char)Serial.read();

      if (isxdigit(userCmd)) {
        collection += userCmd;
        count++;
      }

      if (count == headerSize)
      {
        if (collection.charAt(0) == '7' && collection[1] == '0' && collection[2] == '7' && collection[3] == '1') {

          Serial.print(F("header pass\n"));
        }
        else {
          Serial.print(F("header fail\n"));
          collection = "";
          count = 0;
          return;
        }
      }

      if (count > headerSize)
      {
        unsigned char g_cmd[20] = { 0, };
        memset(g_cmd, 0, sizeof(g_cmd));
        StringManager::change_twoChar_oneHex(g_cmd, (unsigned char*)collection.c_str());

        Serial.println(F("count > 6 "));

        uint8_t u8;


        int max = g_cmd[Command::frame::FRAME_LEN] * 2;

        Serial.print("out max = ");
        Serial.println(max);

        if (max == count)
        {
          Serial.print("in max = ");
          Serial.println(count);

          for (int i = 0; i < max / 2; i++) {
            Serial.print(" ");
            Serial.print(i);
            Serial.print(" : ");
            Serial.print(g_cmd[i], HEX);
          }

          //출력 전용을 cs로 check 비교하기
          byte  cs = StringManager::checksum(g_cmd, (count / 2 - 2)); //1byte용 제대로 안됨.

          byte lowNibble = cs & 0x0F;
          byte highNibble = (cs >> 4) & 0x0F;

          if (lowNibble >= 0x0A)
            lowNibble += ('A' - 0x0A);
          else
            highNibble += '0';

          Serial.print(F("\nCS = "));
          Serial.print(highNibble, HEX);
          Serial.print(lowNibble, HEX);


          Serial.print(F("\ng_cmd cs = "));
          Serial.print(g_cmd[count / 2 - 2], HEX);
          Serial.println(g_cmd[count / 2 - 1], HEX);


          if (highNibble == g_cmd[count / 2 - 2] && lowNibble == g_cmd[count / 2 - 1]) {
            Serial.println(F("Serial Event in"));
            for (int i = 0; i < (count / 2 - 1); i++)
            {
              Serial.print(" ");
              Serial.print(i);
              Serial.print(" : ");
              Serial.print(g_cmd[i], HEX);
              Serial.print(F(" "));

            }
            Serial.println();
            CmdManager cmdManager;
            Serial.println(cmdManager.GetCommand(g_cmd), HEX);

            int * szPins;
            int maxPins;
            switch (cmdManager.GetCommand(g_cmd))
            {
            case Command::led::ON:
              Serial.println(F("Command::led::ON"));
              maxPins = cmdManager.GetDataLen(g_cmd);
              szPins = cmdManager.GetPins(g_cmd);
              for (int i = 0; i < maxPins; i++)
              {
                switch (szPins[i]) {
                case DIGITAL_PINS::TWO:      
                  g_LedLightFlag[DIGITAL_PINS::TWO] = true;        
                break;
                //---------------------------------------------
                case DIGITAL_PINS::THREE:    
                  g_FadeFlag[DIGITAL_PINS::THREE] = false;
                  g_LedLightFlag[DIGITAL_PINS::THREE] = true; 
                break;
                //----------------------------------------------
                case DIGITAL_PINS::FOUR:     
                  g_LedLightFlag[DIGITAL_PINS::FOUR] = true;       
                break;
                //----------------------------------------------
                case DIGITAL_PINS::FIVE:     
                  g_FadeFlag[DIGITAL_PINS::FIVE] = false;
                  g_LedLightFlag[DIGITAL_PINS::FIVE] = true;       
                break;
                //-----------------------------------------------
                case DIGITAL_PINS::SIX:      
                  g_FadeFlag[DIGITAL_PINS::SIX] = false;
                  g_LedLightFlag[DIGITAL_PINS::FIVE] = true;       
                break;
                //-----------------------------------------------
                case DIGITAL_PINS::SEVEN:    
                  g_LedLightFlag[DIGITAL_PINS::SEVEN] = true;      
                break;
                //-----------------------------------------------
                case DIGITAL_PINS::EIGHT:    
                  g_LedLightFlag[DIGITAL_PINS::EIGHT] = true;      
                break;
                //------------------------------------------------
                case DIGITAL_PINS::NINE:     
                  g_FadeFlag[DIGITAL_PINS::NINE] = false;
                  g_LedLightFlag[DIGITAL_PINS::NINE] = true;       
                  
                break;
                case DIGITAL_PINS::TEN:      
                  g_FadeFlag[DIGITAL_PINS::TEN] = false;
                  g_LedLightFlag[DIGITAL_PINS::TEN] = true;        
                break;
                //--------------------------------------------------
                case DIGITAL_PINS::ELEVEN:   
                  g_FadeFlag[DIGITAL_PINS::ELEVEN] = false;
                  g_LedLightFlag[DIGITAL_PINS::ELEVEN] = true;     
                break;
                case DIGITAL_PINS::TWELVE:   
                  g_LedLightFlag[DIGITAL_PINS::TWELVE] = true;     
                break;
                case DIGITAL_PINS::THIRTEEN: 
                  g_LedLightFlag[DIGITAL_PINS::THIRTEEN] = true;   
                break;
                }
              }

              break;
            case Command::led::OFF:
              Serial.println(F("Command::led::OFF"));

              maxPins = cmdManager.GetDataLen(g_cmd);
              szPins = cmdManager.GetPins(g_cmd);

              for (int i = 0; i < maxPins; i++)
              {
                switch (szPins[i]) {
                case DIGITAL_PINS::TWO:      g_LedLightFlag[DIGITAL_PINS::TWO] = false;        break;
                case DIGITAL_PINS::THREE:    g_LedLightFlag[DIGITAL_PINS::THREE] = false;      break;
                case DIGITAL_PINS::FOUR:     g_LedLightFlag[DIGITAL_PINS::FOUR] = false;       break;
                case DIGITAL_PINS::FIVE:     g_LedLightFlag[DIGITAL_PINS::FIVE] = false;       break;
                case DIGITAL_PINS::SIX:      g_LedLightFlag[DIGITAL_PINS::FIVE] = false;       break;
                case DIGITAL_PINS::SEVEN:    g_LedLightFlag[DIGITAL_PINS::SEVEN] = false;      break;
                case DIGITAL_PINS::EIGHT:    g_LedLightFlag[DIGITAL_PINS::EIGHT] = false;      break;
                case DIGITAL_PINS::NINE:     g_LedLightFlag[DIGITAL_PINS::NINE] = false;       break;
                case DIGITAL_PINS::TEN:      g_LedLightFlag[DIGITAL_PINS::TEN] = false;        break;
                case DIGITAL_PINS::ELEVEN:   g_LedLightFlag[DIGITAL_PINS::ELEVEN] = false;     break;
                case DIGITAL_PINS::TWELVE:   g_LedLightFlag[DIGITAL_PINS::TWELVE] = false;     break;
                case DIGITAL_PINS::THIRTEEN: g_LedLightFlag[DIGITAL_PINS::THIRTEEN] = false;   break;
                }
              }

              break;

            case Command::led::FADE_START:
              Serial.println(F("FADE_START"));

              maxPins = cmdManager.GetDataLen(g_cmd);
              szPins = cmdManager.GetPins(g_cmd);

              for (int i = 0; i < maxPins; i++)
              {
                pinMode(szPins[i], 0);
                Serial.print(szPins[i], HEX);

                switch (szPins[i]){
                case DIGITAL_PINS::THREE:  
                  g_LedLightFlag[DIGITAL_PINS::THREE] = false;
                  g_FadeFlag[DIGITAL_PINS::THREE] = true;  
                break;
                case DIGITAL_PINS::FIVE:   
                  g_LedLightFlag[DIGITAL_PINS::FIVE] = false;
                  g_FadeFlag[DIGITAL_PINS::FIVE] = true;   
                break;
                case DIGITAL_PINS::SIX:    
                  g_LedLightFlag[DIGITAL_PINS::SIX] = false;
                  g_FadeFlag[DIGITAL_PINS::SIX] = true;    
                break;
                case DIGITAL_PINS::NINE:   
                  g_LedLightFlag[DIGITAL_PINS::NINE] = false;
                  g_FadeFlag[DIGITAL_PINS::NINE] = true;   
                break;
                case DIGITAL_PINS::TEN:    
                  g_LedLightFlag[DIGITAL_PINS::TEN] = false;
                  g_FadeFlag[DIGITAL_PINS::TEN] = true;    
                break;
                case DIGITAL_PINS::ELEVEN: 
                  g_LedLightFlag[DIGITAL_PINS::ELEVEN] = false;
                  g_FadeFlag[DIGITAL_PINS::ELEVEN] = true; 
                break;
                }
              }

              break;
            case Command::led::FADE_END:
              Serial.println(F("FADE_START"));

              maxPins = cmdManager.GetDataLen(g_cmd);
              szPins = cmdManager.GetPins(g_cmd);

              for (int i = 0; i < maxPins; i++)
              {
                switch (szPins[i]) {
                case DIGITAL_PINS::THREE:  g_FadeFlag[DIGITAL_PINS::THREE] = false;  break;
                case DIGITAL_PINS::FIVE:   g_FadeFlag[DIGITAL_PINS::FIVE] = false;   break;
                case DIGITAL_PINS::SIX:    g_FadeFlag[DIGITAL_PINS::SIX] = false;    break;
                case DIGITAL_PINS::NINE:   g_FadeFlag[DIGITAL_PINS::NINE] = false;   break;
                case DIGITAL_PINS::TEN:    g_FadeFlag[DIGITAL_PINS::TEN] = false;    break;
                case DIGITAL_PINS::ELEVEN: g_FadeFlag[DIGITAL_PINS::ELEVEN] = false; break;
                }
              }
              break;
            }//switch close
          }

          //-------------------------------------------------------
          //이용한 데이터를 초기화
          collection = "";
          count = 0;
        }
      }
      //----------------------------------------------------------------------------

      if (count > maxCnt)
      {
        collection = "";
        count = 0;
      }

    }


  }
};

//long SerialCollector::interval = 800;
//long SerialCollector::perviousMillis = 0;
int SerialCollector::count = 0;
String SerialCollector::collection = "";


void setup() {
  Serial.begin(9600); //1bd == 1bps  => 9600bd / 8bit = 1200 character

  //Digital pins => general pins
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(7, OUTPUT);

  //Digital pins => PWM pins
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);

  //Timer1.initialize(100);
  //Timer1.attachInterrupt(timerSerial);
}
void serialEvent()
{
  SerialCollector serialCollector;
  serialCollector.start();
}

void LED_LIGHT_CONTROL(int pin, boolean isRun)
{
  if (isRun) digitalWrite(pin, HIGH);
  else digitalWrite(pin, LOW);
}

void LED_LIGHT_RUN()
{
  LED_LIGHT_CONTROL(DIGITAL_PINS::TWO,g_LedLightFlag[DIGITAL_PINS::TWO]);
  LED_LIGHT_CONTROL(DIGITAL_PINS::THREE,g_LedLightFlag[DIGITAL_PINS::THREE]);
  LED_LIGHT_CONTROL(DIGITAL_PINS::FOUR,g_LedLightFlag[DIGITAL_PINS::FOUR]);
  LED_LIGHT_CONTROL(DIGITAL_PINS::FIVE,g_LedLightFlag[DIGITAL_PINS::FIVE]);
  LED_LIGHT_CONTROL(DIGITAL_PINS::SIX,g_LedLightFlag[DIGITAL_PINS::SIX]);
  LED_LIGHT_CONTROL(DIGITAL_PINS::SEVEN,g_LedLightFlag[DIGITAL_PINS::SEVEN]);
  LED_LIGHT_CONTROL(DIGITAL_PINS::EIGHT,g_LedLightFlag[DIGITAL_PINS::EIGHT]);
  LED_LIGHT_CONTROL(DIGITAL_PINS::NINE,g_LedLightFlag[DIGITAL_PINS::NINE]);
  LED_LIGHT_CONTROL(DIGITAL_PINS::TEN,g_LedLightFlag[DIGITAL_PINS::TEN]);
  LED_LIGHT_CONTROL(DIGITAL_PINS::ELEVEN,g_LedLightFlag[DIGITAL_PINS::ELEVEN]);
  LED_LIGHT_CONTROL(DIGITAL_PINS::TWELVE,g_LedLightFlag[DIGITAL_PINS::TWELVE]);
  LED_LIGHT_CONTROL(DIGITAL_PINS::THIRTEEN,g_LedLightFlag[DIGITAL_PINS::THIRTEEN]);
}

void loop() {
  deviceRun();
}

void deviceRun() {
  LED_LIGHT_RUN();

  //allFading(); //작동 안할때도 마구 탄다. for문 102번

  fading_Three_SuperLoop(g_FadeFlag[DIGITAL_PINS::THREE]);
  fading_Five_SuperLoop(g_FadeFlag[DIGITAL_PINS::FIVE]);
  fading_Six_SuperLoop(g_FadeFlag[DIGITAL_PINS::SIX]);

}

void allFading()
{
  for (int fadeValue = 0; fadeValue <= 255; fadeValue += 5) {
    if (g_FadeFlag[DIGITAL_PINS::THREE])
      analogWrite(DIGITAL_PINS::THREE, fadeValue);
    if (g_FadeFlag[DIGITAL_PINS::FIVE])
      analogWrite(DIGITAL_PINS::FIVE, fadeValue);
    if (g_FadeFlag[DIGITAL_PINS::SIX])
      analogWrite(DIGITAL_PINS::SIX, fadeValue);
    delay(30);
  }
  for (int fadeValue = 255; fadeValue >= 0; fadeValue -= 5) {
    if (g_FadeFlag[DIGITAL_PINS::THREE])
      analogWrite(DIGITAL_PINS::THREE, fadeValue);
    if (g_FadeFlag[DIGITAL_PINS::FIVE])
      analogWrite(DIGITAL_PINS::FIVE, fadeValue);
    if (g_FadeFlag[DIGITAL_PINS::SIX])
      analogWrite(DIGITAL_PINS::SIX, fadeValue);
    delay(30);
  }

}

void FadingOne(int ledPin)
{
  for (int fadeValue = 0; fadeValue <= 255; fadeValue += 5) {
    analogWrite(ledPin, fadeValue);
    delay(30);
  }
  for (int fadeValue = 255; fadeValue >= 0; fadeValue -= 5) {
    analogWrite(ledPin, fadeValue);
    delay(30);
  }
}

void fadingMulti(int pin, int& brightness, int& fadeAmount)
{
  analogWrite(pin, brightness);

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }
}


void fading_Three_SuperLoop(bool beginning)
{

  long fadingThree_inverval = 20;
  static long fadingThree_previousMillis = 0;
  static int brightness_three = 0;
  static int fadeAmount_three = 5;


  unsigned long currentMillis = millis();

  if (currentMillis - fadingThree_previousMillis >= fadingThree_inverval) {
    fadingThree_previousMillis = currentMillis;


    if (beginning == true)
    {
      //FadingOne(DIGITAL_PINS::THREE);
      fadingMulti(DIGITAL_PINS::THREE, brightness_three, fadeAmount_three);
    }
    else {
      analogWrite(DIGITAL_PINS::THREE, 0);
    }
  }
}

void fading_Five_SuperLoop(bool beginning)
{
  long fadingFive_inverval = 30;
  static long fadingFive_previousMillis = 0;
  static int brightness_five = 0;
  static int fadeAmount_five = 5;

  unsigned long currentMillis = millis();

  if (currentMillis - fadingFive_previousMillis >= fadingFive_inverval) {
    fadingFive_previousMillis = currentMillis;

    if (beginning == true)
    {
      //FadingOne(DIGITAL_PINS::FIVE);
      fadingMulti(DIGITAL_PINS::FIVE, brightness_five, fadeAmount_five);
    }
    else {
      analogWrite(DIGITAL_PINS::FIVE, 0);
    }
  }
}

void fading_Six_SuperLoop(bool beginning)
{

  long fadingSix_inverval = 40;
  static long fadingSix_previousMillis = 0;
  static int brightness_six = 0;
  static int fadeAmount_six = 5;

  unsigned long currentMillis = millis();

  if (currentMillis - fadingSix_previousMillis >= fadingSix_inverval) {
    fadingSix_previousMillis = currentMillis;

    if (beginning == true)
    {
      //FadingOne(DIGITAL_PINS::SIX);
      fadingMulti(DIGITAL_PINS::SIX, brightness_six, fadeAmount_six);
    }
    else {
      analogWrite(DIGITAL_PINS::SIX, 0);
    }
  }
}

void fading_Nine_SuperLoop(bool beginning)
{

  long fadingNine_inverval = 40;
  static long fadingNine_previousMillis = 0;
  static int brightness_nine = 0;
  static int fadeAmount_nine = 5;

  unsigned long currentMillis = millis();

  if (currentMillis - fadingNine_previousMillis >= fadingNine_inverval) {
    fadingNine_previousMillis = currentMillis;

    if (beginning == true)
    {
      //FadingOne(DIGITAL_PINS::NINE);
      fadingMulti(DIGITAL_PINS::NINE, brightness_nine, fadeAmount_nine);
    }
    else {
      analogWrite(DIGITAL_PINS::NINE, 0);
    }
  }
}

void fading_Ten_SuperLoop(bool beginning)
{
  long fadingTen_inverval = 40;
  static long fadingTen_previousMillis = 0;
  static int brightness_ten = 0;
  static int fadeAmount_ten = 5;

  unsigned long currentMillis = millis();

  if (currentMillis - fadingTen_previousMillis >= fadingTen_inverval) {
    fadingTen_previousMillis = currentMillis;

    if (beginning == true)
    {
      //FadingOne(DIGITAL_PINS::TEN);
      fadingMulti(DIGITAL_PINS::TEN, brightness_ten, fadeAmount_ten);
    }
    else {
      analogWrite(DIGITAL_PINS::TEN, 0);
    }
  }
}

void fading_Eleven_SuperLoop(bool beginning)
{
  long fadingEleven_inverval = 40;
  static long fadingEleven_previousMillis = 0;
  static int brightness_eleven = 0;
  static int fadeAmount_eleven = 5;

  unsigned long currentMillis = millis();

  if (currentMillis - fadingEleven_previousMillis >= fadingEleven_inverval) {
    fadingEleven_previousMillis = currentMillis;

    if (beginning == true)
    {
      //FadingOne(DIGITAL_PINS::ELEVEN);
      fadingMulti(DIGITAL_PINS::ELEVEN, brightness_eleven, fadeAmount_eleven);
    }
    else {
      analogWrite(DIGITAL_PINS::ELEVEN, 0);
    }
  }
}