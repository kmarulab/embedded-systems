/*
 * Copyright (c) 2017-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <chrono>
#include <cmath>
#include <vector>

// Initialize a pins to perform analog input and digital output functions
AnalogIn   ain(A0);
DigitalOut dout(ARDUINO_UNO_D8);

using namespace std::chrono;

BufferedSerial pc(USBTX,USBRX);
char buff[6]; // largest value is 65535, new line

Ticker sample;

std::vector< float > datapoints;
std::vector< float > times;
int numPoints = 0; // counting number of points

float t1 = -1;
float t2 = -1;
bool onPeak = false;
int maxIndex = 1;
float maxValue = 0;
bool calculated = true;
int threshold = 60000;

void sampleADC(){
    unsigned short value = ain.read_u16();

    // buff[0] = value/10000 + 0x30;
    // value = value - (buff[0]-0x30)*10000;
    
    // buff[1] = value/1000 + 0x30;
    // value = value - (buff[1]-0x30)*1000;
    
    // buff[2] = value/100 + 0x30;
    // value = value - (buff[2]-0x30)*100;
    
    // buff[3] = value/10 + 0x30;
    // value = value - (buff[3]-0x30)*10;
    
    // buff[4] = value/1 + 0x30;
    
    // printf("%d\n",value);
    datapoints.push_back(value);
    times.push_back(float(numPoints)/(100));
    numPoints++;
    if (value > threshold){
        if (onPeak == false) {
            maxIndex = numPoints;
            maxValue = value;
            onPeak = true;
        }
        else{
            if (onPeak == true){
                calculated = false;
                onPeak = false;
            }
        }
    }
        
    if (onPeak == true){
        if (value > maxValue){
            maxValue = value;
            maxIndex = numPoints;
        }
    }

    if (calculated == false){
        if (t1 >= 0){
            t2 = times[maxIndex];
            printf("IHR = %.1f\n",1/(t2 - t1)*60);
            printf("t1 = %.1f\n",t1);
            printf("t2 = %.1f\n\n",t2);
            t1 = t2;
        }
        else{
            t1 = times[maxIndex];
            }
        calculated = true;
    }
    // pc.write(&buff,6);
}

int main(void)
{
    // needed to use thread_sleep_for in debugger
    // your board will get stuck without it :(
    #if defined(MBED_DEBUG) && DEVICE_SLEEP
        HAL_DBGMCU_EnableDBGSleepMode();
    #endif
    
    buff[5] = '\n';
    sample.attach(&sampleADC,10ms);

    while (1) {
        thread_sleep_for(100);
        if(buff[0]>='5' && buff[1]>='5'){
            dout = true;
        }
        else{
            dout = false;
        }
    }
}