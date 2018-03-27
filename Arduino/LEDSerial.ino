//-----------------------------------------------------------------------------------
//전역 변수

enum DIGITAL_PINS { TWO = 2, THREE = 3, FOUR = 4, FIVE = 5, SIX = 6, SEVEN = 7, EIGHT = 8, NINE = 9, TEN = 10, ELEVEN = 11, TWELVE = 12, THIRTEEN = 13 };
//enum PWM_DPINS { FADE_THREE = 3, FADE_FIVE = 5, FADE_SIX = 6, FADE_NINE = 9, FADE_TEN = 10, FADE_ELEVEN = 11 };

int PWM_DPINS[6] = { DIGITAL_PINS::THREE, DIGITAL_PINS::FIVE, DIGITAL_PINS::SIX, DIGITAL_PINS::NINE, DIGITAL_PINS::TEN, DIGITAL_PINS::ELEVEN };

enum virtualPin { one = 0, two = 1, three = 2, four = 3, five = 4, six = 5 };


const int MAX_DIGITAL_VIRTUAL = 12 + 2; //0,1 번째 인덱스는 사용하지 않음.
boolean g_FadeFlag[MAX_DIGITAL_VIRTUAL] = { false, };
boolean g_LedLightFlag[MAX_DIGITAL_VIRTUAL] = { false, };

int g_Cmd = 0;

//---------------------------------------------------------

//초기화(아두이노 기본 함수)
void setup();

//아두이노 제공 기본 함수
void serialEvent();

//루프(아두이노 기본 함수)
void loop();

//----------------------------------------------------------------------------
class Command
{
public:
  virtual void execute();
};

class LED_FADE : public Command
{
private:
  static int brightness;
  static int fadeAmount;
public:
  void control(int pin, boolean isRun);
  void execute();
};

int LED_FADE::brightness = 0;
int LED_FADE::fadeAmount = 5;

void LED_FADE::execute()
{
  control(DIGITAL_PINS::THREE,g_FadeFlag[DIGITAL_PINS::THREE]);
  control(DIGITAL_PINS::FIVE,g_FadeFlag[DIGITAL_PINS::FIVE]);
  control(DIGITAL_PINS::SIX,g_FadeFlag[DIGITAL_PINS::SIX]);
}

void LED_FADE::control(int pin, boolean isRun)
{
  if(isRun){
    analogWrite(pin, brightness);
  }else{
    analogWrite(pin, 0);
  }

  brightness = brightness + fadeAmount;

  if (brightness <= 0 || brightness >= 255) {
     fadeAmount = -fadeAmount;
  } 
  delay(30);
}

class LED_LIGHT : public Command
{
public:
  void control(int pin, boolean isRun);
  void execute();
};


void LED_LIGHT::control(int pin, boolean isRun)
{
  if (isRun) digitalWrite(pin, HIGH);
  else digitalWrite(pin, LOW);
}

void LED_LIGHT::execute()
{
  control(DIGITAL_PINS::TWO, g_LedLightFlag[DIGITAL_PINS::TWO]);
  control(DIGITAL_PINS::THREE, g_LedLightFlag[DIGITAL_PINS::THREE]);
  control(DIGITAL_PINS::FOUR, g_LedLightFlag[DIGITAL_PINS::FOUR]);
  control(DIGITAL_PINS::FIVE, g_LedLightFlag[DIGITAL_PINS::FIVE]);
  control(DIGITAL_PINS::SIX, g_LedLightFlag[DIGITAL_PINS::SIX]);
  control(DIGITAL_PINS::SEVEN, g_LedLightFlag[DIGITAL_PINS::SEVEN]);
  control(DIGITAL_PINS::EIGHT, g_LedLightFlag[DIGITAL_PINS::EIGHT]);
  control(DIGITAL_PINS::NINE, g_LedLightFlag[DIGITAL_PINS::NINE]);
  control(DIGITAL_PINS::TEN, g_LedLightFlag[DIGITAL_PINS::TEN]);
  control(DIGITAL_PINS::ELEVEN, g_LedLightFlag[DIGITAL_PINS::ELEVEN]);
  control(DIGITAL_PINS::TWELVE, g_LedLightFlag[DIGITAL_PINS::TWELVE]);
  control(DIGITAL_PINS::THIRTEEN, g_LedLightFlag[DIGITAL_PINS::THIRTEEN]);
}

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
    int slen = strlen((char*)src) / sizeof(unsigned char);
    if (slen <= 0) return;

    int maxSize = (slen % 2 == 0) ? slen / 2 : slen / 2 + 1;  // 홀수 일때 남는 데이터가 한개 임으로

    int lastindex = 0;
    for (int i = 0, count = 0; i < maxSize; i++, count += 2) {
      if (i == (maxSize - 1) && (slen % 2)) {
        dest[i] = GetAsciiToNumber(src[count]);
        break;
      }
      dest[i] = ((GetAsciiToNumber(src[count]) << 4) + GetAsciiToNumber(src[count + 1]));
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

//새로운 명령어 나오면 추가해야 되는 곳
enum formal { HEADER1 = 0x70, HEADER2 = 0x71 };
enum led { ON = 0x50, OFF = 0x51, FADE_START = 0x60, FADE_END = 0x61 };
enum frame { HEAD1 = 0, HEAD2 = 1, CMD = 2, FRAME_LEN = 3, DATA_LEN = 4, PINNUM_ONE = 5, PINNUM_TWO = 6, PINNUM_THREE = 7, PINNUM_FOUR = 8, PINNUM_FIVE = 9, PINNUM_SIX = 10 }; //현재 LEN은 사용되지 않음.


class CmdManager
{
public:
  static bool CheckHeader(unsigned char *serial) {
    if (serial[frame::HEAD1] == formal::HEADER1 && serial[frame::HEAD2] == formal::HEADER2)
    {
      return true;
    }
    return false;
  }

  //새로운 명령어 나오면 추가해야 되는 곳
  unsigned char GetCommand(unsigned char *serialCmd) {
    if (serialCmd[frame::HEAD1] != formal::HEADER1 || serialCmd[frame::HEAD2] != formal::HEADER2) return 0;

    switch (serialCmd[frame::CMD]) {
    case led::ON:
      return led::ON;
      break;
    case led::OFF:
      return led::OFF;
      break;
    case led::FADE_START:
      return led::FADE_START;
      break;
    case led::FADE_END:
      return led::FADE_END;
      break;
    }

    return 0;
  }
  int GetFrameLen(unsigned char* serialCmd) {
    return serialCmd[frame::FRAME_LEN];
  }
  unsigned char GetDataLen(unsigned char* serialCmd) {
    return serialCmd[frame::DATA_LEN];
  }
  int * GetPins(unsigned char* serialCmd) {

    int pinCounts = GetDataLen(serialCmd);

    Serial.print(F("pinCounts = "));
    Serial.println(pinCounts);

    int pins[10];

    for (int i = 0; i < pinCounts; i++) {
      pins[i] = serialCmd[(frame::PINNUM_ONE) + i];

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
  static String collection;
  static int count;
  enum check { HEADER1, HEADER2, COMMAND, };
  const int maxCnt = 40;
  const int headerSize = 8; //4 * 2 (현재는 2개의 문자가 한개의 hex문자)
public:
  void start() {
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
        unsigned char cmd[20] = { 0, };
        memset(cmd, 0, sizeof(cmd));
        StringManager::change_twoChar_oneHex(cmd, (unsigned char*)collection.c_str());

        uint8_t u8;
        int max = cmd[frame::FRAME_LEN] * 2;

        if (max == count)
        {
          //출력 전용을 cs로 check 비교하기
          byte  cs = StringManager::checksum(cmd, (count / 2 - 2)); //1byte용 제대로 안됨.

          byte lowNibble = cs & 0x0F;
          byte highNibble = (cs >> 4) & 0x0F;

          if (lowNibble >= 0x0A)
            lowNibble += ('A' - 0x0A);
          else
            highNibble += '0';

          if (highNibble == cmd[count / 2 - 2] && lowNibble == cmd[count / 2 - 1]) {
            CmdManager cmdManager;
            Serial.println(cmdManager.GetCommand(cmd), HEX);

            int * szPins;
            int maxPins;

            g_Cmd = cmdManager.GetCommand(cmd);
            switch (g_Cmd)
            {
            case led::ON:
              maxPins = cmdManager.GetDataLen(cmd);
              szPins = cmdManager.GetPins(cmd);
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
                  g_LedLightFlag[DIGITAL_PINS::SIX] = true;
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
            case led::OFF:

              maxPins = cmdManager.GetDataLen(cmd);
              szPins = cmdManager.GetPins(cmd);

              for (int i = 0; i < maxPins; i++)
              {
                switch (szPins[i]) {
                case DIGITAL_PINS::TWO:      g_LedLightFlag[DIGITAL_PINS::TWO] = false;        break;
                case DIGITAL_PINS::THREE:    g_LedLightFlag[DIGITAL_PINS::THREE] = false;      break;
                case DIGITAL_PINS::FOUR:     g_LedLightFlag[DIGITAL_PINS::FOUR] = false;       break;
                case DIGITAL_PINS::FIVE:     g_LedLightFlag[DIGITAL_PINS::FIVE] = false;       break;
                case DIGITAL_PINS::SIX:      g_LedLightFlag[DIGITAL_PINS::SIX] = false;       break;
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

            case led::FADE_START:

              maxPins = cmdManager.GetDataLen(cmd);
              szPins = cmdManager.GetPins(cmd);

              for (int i = 0; i < maxPins; i++)
              {
                pinMode(szPins[i], 0);
                Serial.print(szPins[i], HEX);

                switch (szPins[i]) {
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
            case led::FADE_END:

              maxPins = cmdManager.GetDataLen(cmd);
              szPins = cmdManager.GetPins(cmd);

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
int SerialCollector::count = 0;
String SerialCollector::collection = "";

void setup() {
  Serial.begin(9600); //1bd == 1bps  => 9600bd / 8bit = 1200 character

  //Digital pins => general pins
  // pinMode(2, OUTPUT);
  //pinMode(4, OUTPUT);
  // pinMode(7, OUTPUT);

  //Digital pins => PWM pins
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
}
void serialEvent()
{
  SerialCollector serialCollector;
  serialCollector.start();
}

void loop() {
  deviceRun();
}

Command * cmdProgram = NULL;

void deviceRun() {
  switch (g_Cmd)
  {
  case led::ON:
  case led::OFF:
    if (cmdProgram == NULL) {
      cmdProgram = new LED_LIGHT();
    }
    else {
      delete cmdProgram;
      cmdProgram = new LED_LIGHT();
    }
    break;
  case led::FADE_START:
  case led::FADE_END:
    if (cmdProgram == NULL) {
      cmdProgram = new LED_FADE();
    }
    else {
      delete cmdProgram;
      cmdProgram = new LED_FADE();
    }
    break;
  }

  if (cmdProgram != NULL)
    cmdProgram->execute();
}