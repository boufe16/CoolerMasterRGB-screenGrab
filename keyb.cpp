// keyb.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <SDKDLL.h>
#include <conio.h> 
#include <string>
#include <math.h>
#include <thread>
#include <vector>
#include <mutex>          // std::mutex
KEY_COLOR keyColour(0,0,0);
//declare our function
void sampleColor(HDC *hdc_, int posX, int posY);
std::mutex mtx;           // mutex for critical section
struct xyPos {
	int x; 
	int y; 
};
float Rtot, Gtot, Btot;
xyPos posArray[100];

int main() {
	COLOR_MATRIX myMtrx;
	std::cout << "whats up" << std::endl;
	std::string input = "";
	SetControlDevice(DEV_MKeys_L);
	HDC hdc_ = GetDC(GetDesktopWindow());
	
	// "store" all threads here, 
	std::vector<std::thread> threads;

	//check if device is plugged in... 
	if (IsDevicePlug()){
		if (EnableLedControl(true)) {
		//controlled by software or firmware
		
			int key_code=0; 
			
			while (key_code!=27) {
				Rtot = Gtot= Btot= 0;
				 
				for (int i = 0; i < 4; i++) {
					for (int j = 0; j < 2; j++) {
						//on a thread sample the color at position XY 
						threads.push_back(std::thread(sampleColor, &hdc_, i, j));
					}
				}

				// when all the threads return, we can calculate our "mean"
				// colour value 
				for (auto& th : threads) {
					th.join();
				}
				threads.clear();

				Rtot = Rtot / 8.0;
				Gtot = Gtot / 8.0;
				Btot = Btot / 8.0;
				keyColour.r  = (int)(0.8*sqrt(Rtot)+ 0.2*keyColour.r);
				keyColour.g  = (int)(0.8*sqrt(Gtot)+ 0.2*keyColour.g);
				keyColour.b  = (int)(0.8*sqrt(Btot)+ 0.2*keyColour.b);
				if (!SetFullLedColor(keyColour.r, keyColour.g, keyColour.b))
				{
					std::cout << "shit's not working mY DudE" << std::endl;
				}
				if (_kbhit()) {
					key_code = _getch();
				    // do stuff depending on key_code
					std::cout << key_code << std::endl;
				}
				else
					continue;
			}
		}
		else {
			std::cout << "Failed to enable software control"<< std::endl;
		}

		
	}
	else {
		std::cout << "Device NOT found. Plug keyboard in and try again" << std::endl;
	}
	std::cout << "Press ENTER to exit..." << std::endl;
	std::getline(std::cin, input);
	//release DC 
	ReleaseDC(GetDesktopWindow(), hdc_);
	return 0;
}

void sampleColor(HDC *hdc_, int posX, int posY) {
	mtx.lock();

	COLORREF clr = GetPixel(*hdc_, 512 + 512 * posX, 480 + 480 * posY);
	mtx.unlock();

	int R,G,B;
	R = GetRValue(clr);
	G = GetGValue(clr);
	B = GetBValue(clr);
	Rtot += R * R;
	Gtot += G * G;
	Btot += B * B;
}