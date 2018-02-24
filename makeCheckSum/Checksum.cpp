//CheckSum make
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstdio>
#include <cstring>


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
			//default: return 0; break;
		default: return (apabetNum - '0'); break;
		}
	}
public:
	static void change_twoChar_oneHex(unsigned char * dest, unsigned char * src) {
		printf("change_twoChar_oneHex start");
		int slen = strlen((char*)src) / sizeof(unsigned char);
		if (slen <= 0) return;

		int maxSize = (slen % 2 == 0) ? slen / 2 : slen / 2 + 1;  // 홀수 일때 남는 데이터가 한개 임으로
		printf("MaxSize = %d", maxSize);

		int lastindex = 0;
		for (int i = 0, count = 0; i < maxSize; i++, count += 2) {
			if (i == (maxSize - 1) && (slen % 2)) {
				dest[i] = GetAsciiToNumber(src[count]);
				printf("dest[%d] = %x", i,dest[i]);
				break;
			}
			dest[i] = ((GetAsciiToNumber(src[count]) << 4) + GetAsciiToNumber(src[count + 1]));

			printf("dest[%d] = %x", i,dest[i]);
		}
		printf("\n");
		for (int i = 0; i < maxSize; i++) {
			printf("%x ", dest[i]);
		
			//if ((i + 1) % 8 == 0) Serial.println();
			// if (i == maxSize - 1) Serial.println();
		}
		printf("");
	}

	static unsigned int * checksum(unsigned char arr[], int size)
	{
		unsigned int cs = 0;
		int length = strlen((char*)arr);
		for (int i = 0; i < length; i++)
			cs += arr[i];

		unsigned int arrNibble[2];

		unsigned int lowNibble = cs & 0x0F;
		unsigned int highNibble = (cs >> 4) & 0x0F;

		if (lowNibble >= 0x0A)
			lowNibble += ('A' - 0x0A);
		else
			highNibble += '0';

		arrNibble[0] = highNibble;
		arrNibble[1] = lowNibble;

		return arrNibble;
	}
};

int main()
{
	//header 2byte, command 1byte, length = 1byte, pinNumber = 1byte

	uint8_t hexString[][20] = 
	{ 
		"7071500A03030506", //0443 //7071500A030305060443
		"7071510A03030506",
		"7071600A03030506",
		"7071610A03030506",
	};
	uint8_t oneHexString[20];
	
	

	for (int i = 0; i < sizeof(hexString) / sizeof(hexString[0]); i++)
	{
		memset(oneHexString, 0, sizeof(oneHexString) / sizeof(uint8_t));
		StringManager::change_twoChar_oneHex(oneHexString, hexString[i]);
		printf("\nsize = %d\n", strlen((char*)hexString));

		unsigned int * cs = StringManager::checksum(oneHexString, strlen((char*)hexString));
		//printf("%x\n", StringManager::checksum(oneHexString, strlen((char*)hexString)));
		printf("%02x %02x\n", cs[0],cs[1]);
	}
	return 0;
}