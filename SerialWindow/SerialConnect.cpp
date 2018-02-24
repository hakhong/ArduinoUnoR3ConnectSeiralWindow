//https://playground.arduino.cc/Interfacing/CPPWindows base
#include "SerialClass.h"	
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <windows.h>
#include <process.h>
#include <future>
#include <vector>
#include "comSearch.h"

#define NUM_THREAD   2

unsigned char g_cmd[][3][30] =
{
	{	
		/*
		 7071 50 0A 03 03 05 06 0443
		 header(2byte): 7071 
		 command(1byte) : 50  
		 strLen(1byte) : 0A => 1byte hex 10  
		 pinCount(1byte) : 03 => 1byte hex 3 
		 pinNumber(3byte) : 03 pin, 05 pin 06 pin 
		 checksum(2byte) : 0443 
		*/
		//LED ON
		{"7071500A030305060443"},
	},
	{	//LED OFF
		{"7071510A030305060444"},
	},
	{	//LED FADE START
		{"7071600A030305060543"}
	},
	{	//LED FADE END
		{ "7071610A030305060544" }
	},
};

/*
std::vector<char> vszkeyBuf;

void inputKey()
{
	while (1)
	{
		char cmd;
		scanf("%c", &cmd);
		vszkeyBuf.push_back(cmd);
	}
}
*/

// application reads from the specified serial port and reports the collected data
int _tmain(int argc, _TCHAR* argv[])
{
	//std::thread first(inputKey);
	
	printf("Welcome to the serial test app!\n\n");


	Serial* SP = new Serial("\\\\.\\COM3");    // adjust as needed

	if (SP->IsConnected()) {
		printf("We're connected");

		std::future<int> read = std::async(std::launch::async, [&SP] { 
			int i;

			char incomingData[513] = "";			// don't forget to pre-allocate memory
													//printf("%s\n",incomingData);
			int dataLength = 512;
			int readResult = 0;

			while (SP->IsConnected()) {
					readResult = SP->ReadData(incomingData, dataLength);
					// printf("Bytes read: (0 means no data available) %i\n",readResult);
					incomingData[readResult] = 0;
					printf("%s", incomingData);
					//Sleep(300);
					Sleep(200);
			}
			return 1;
		});

		std::future<int> write = std::async(std::launch::async, [&SP] { 
			while (SP->IsConnected()) {
				unsigned char sendData[80] = "";

				char cmd;
				
				/*
				if (vszkeyBuf.size() > 0)
				{
					cmd = vszkeyBuf.at(vszkeyBuf.size()-1);
					vszkeyBuf.pop_back();
				}
				*/
				scanf("%c", &cmd);

				int szCmd = sizeof(g_cmd) / sizeof(g_cmd[0]);
				
				switch (cmd)
				{
				case '1':
					szCmd = 1; 
					
					for (int i = 0; i < szCmd; i++) {
					    Sleep(300);

						SP->WriteData((char*)g_cmd[0][i], strlen((char*)g_cmd[0][i]));


						for (int j = 0; j < strlen((char*)g_cmd[0][i]); j++) {
							printf("%02x ", g_cmd[0][i][j]);
						}
						printf("\n");
					
			
					}
					break;
				case '2':
					szCmd = 1; //임시
					for (int i = 0; i < szCmd; i++) {
						Sleep(300);
						SP->WriteData((const char*)g_cmd[1][i], strlen((const char*)g_cmd[1][i]));

						for (int j = 0; j < 10; j++) {
							printf("%02x ", (char*)g_cmd[0][i][j]);
						}
						printf("\n");
					}


					break;

				case '3':
					szCmd = 1;
					for (int i = 0; i < szCmd; i++) {
						//Sleep(2000);
						Sleep(300);
						SP->WriteData((const char*)g_cmd[2][i], strlen((const char*)g_cmd[2][i]));
						//Sleep(3000);

						for (int j = 0; j < 10; j++) {
							printf("%02x ", (char*)g_cmd[0][i][j]);
						}
						printf("\n");
					}
					break;

				case '4':
					for (int i = 0; i < szCmd; i++) {
						Sleep(300);
						SP->WriteData((const char*)g_cmd[3][i], strlen((const char*)g_cmd[3][i]));
						//Sleep(3000);

						for (int j = 0; j < 10; j++) {
							printf("%02x ", (char*)g_cmd[0][i][j]);
						}
						printf("\n");
					}

					break;
				}
				Sleep(1000); //1000 m/s 이하로 줄 경우 , 빠르게 데이터가 입력되었을때 아두이노 내부에서 처리 못하고 원래 상태로 복귀도 못함. 
			}
			return 1;
		
		});

	}

	//first.join();
	return 0;
}
