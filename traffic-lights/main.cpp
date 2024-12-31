/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <chrono>
#include <vector>

// example setup for echo and trigger using D2 and D3
DigitalIn echo1(ARDUINO_UNO_D10);
DigitalIn echo2(ARDUINO_UNO_D12);
DigitalOut trigger1(ARDUINO_UNO_D11);
DigitalOut trigger2(ARDUINO_UNO_D13);

// timer for echo code
Timer echoDurationOne;
Timer echoDurationTwo;
using namespace std::chrono; // namespace for timers

enum states {init, redLane, blueLane, redLaneTimed, blueLaneTimed, yellowLight, redLights} state;
int red, blue, numTicks, fromRed;
vector<float> movAvgOne;
vector<float> movAvgTwo;
float avgOne;
float avgTwo;

BusOut redLight(ARDUINO_UNO_D4,ARDUINO_UNO_D3,ARDUINO_UNO_D2);
BusOut blueLight(ARDUINO_UNO_D7,ARDUINO_UNO_D6,ARDUINO_UNO_D5);

Ticker lights;
Ticker sensors;

// defines to make our lives easier when using busses
// can use same defines for light1 and light2 as long as wiring is consistent
#define RED 0b100 // top bit / pin is red
#define YELLOW 0b010 // middle bit / pin is yellow 
#define GREEN 0b001 // lowest bit / pin is green

float getDistanceOne(void){
    // trigger pulse
    float distance;
    trigger1 = 1;
    wait_us(10);
    trigger1 = 0;

    while (echo1 != 1); // wait for echo to go high
    echoDurationOne.start(); // start timer
    while (echo1 == 1);
    echoDurationOne.stop(); // stop timer

    // print distance in cm (switch to divison by 148 for inches)
    distance = (float)duration_cast<microseconds>(echoDurationOne.elapsed_time()).count()/58.0;
    echoDurationOne.reset(); // need to reset timer (doesn't happen automatically)
    return distance;
}
float getDistanceTwo(void){
    // trigger pulse
    float distance2;
    trigger2 = 1;
    wait_us(10);
    trigger2 = 0;

    while (echo2 != 1); // wait for echo to go high
    echoDurationTwo.start(); // start timer
    while (echo2 == 1);
    echoDurationTwo.stop(); // stop timer

    // print distance in cm (switch to divison by 148 for inches)
    distance2 = (float)duration_cast<microseconds>(echoDurationTwo.elapsed_time()).count()/58.0;
    echoDurationTwo.reset(); // need to reset timer (doesn't happen automatically)
    return distance2;
}

void checkCars(void){
    float distOne = getDistanceOne();
    int sizeOne = movAvgOne.size();
    if(sizeOne >= 20){
        avgOne -= movAvgOne[0]/20;
        movAvgOne.erase(movAvgOne.begin());
    }
    avgOne += distOne/20;
    movAvgOne.push_back(distOne);
    
    float distTwo = getDistanceTwo();
    int sizeTwo = movAvgTwo.size();
    if(sizeTwo >= 20){
        avgTwo -= movAvgTwo[0]/20;
        movAvgTwo.erase(movAvgTwo.begin());
    }
    avgTwo += distTwo/20;
    movAvgTwo.push_back(distTwo);

    if(sizeOne == 20){
        if (avgOne < 25){
            if(red == 0){
                printf("Red Car Detected\n");
            }
            red = 1;
        }else{
            if(red == 1){
                printf("Red Car Gone\n");
            }
            red = 0;
        } 
    }
    
    if(sizeTwo == 20){
        if (avgTwo < 25){
            if(blue == 0){
                printf("Blue Car Detected\n");
            }
            blue = 1;
        }else{
            if(blue == 1){
                printf("Blue Car Gone\n");
            }
            blue = 0;
        }
    }

    // printf("avgOne = %f\n",avgOne);
    // printf("avgTwo = %f\n",avgTwo);
}

void checkLights(void){
    switch(state) {
        case init: 
            redLight = 0;
            blueLight = 0;
            numTicks = 0;
            fromRed = 0;

            if(!blue){
                state = redLane;
            }
            else if(!red && blue){
                state = blueLane;
            }
            else if(red && blue){
                state = redLaneTimed;
            }
        break;

        case redLane:
            printf("Red Lane\n");
            redLight = GREEN;
            blueLight = RED;
            if(red && blue){
                state = redLaneTimed;
                numTicks = 0;
            }
            else if(blue){
                state = yellowLight;
                redLight = YELLOW;
                numTicks = 0;
            }
        break;
        case blueLane:
            printf("Blue Lane\n");
            redLight = RED;
            blueLight = GREEN;
            if(!(!red && blue)){
                state = yellowLight;
                blueLight = YELLOW;
                numTicks = 0;
            }
        break;
        case redLaneTimed:
            printf("Red Lane Timed\n");
            redLight = GREEN;
            blueLight = RED;
            if(numTicks >= 120 || !red){
                state = yellowLight;
                redLight = YELLOW;
                numTicks = 0;
                fromRed = 1;
            }
            else if(numTicks < 120){
                numTicks++;
            }
        break;
        case blueLaneTimed:
            printf("Blue Lane Timed\n");
            redLight = RED;
            blueLight = GREEN;
            if(numTicks >= 60 || !blue){
                state = yellowLight;
                blueLight = YELLOW;
                numTicks = 0;
                fromRed = 0;
            }
            else if(numTicks < 60){
                numTicks++;
            }
        break;
        case yellowLight:
            printf("Yellow Light\n");
            if(numTicks >= 5){
                state = redLights;
                redLight = RED;
                blueLight = RED;
                numTicks = 0;
            }
            else if(numTicks < 5){
                numTicks++;
            }
        break;
        case redLights:
            printf("Red Light\n");
            if(numTicks >= 5){
                if(!blue){
                    state = redLane;
                    fromRed = 0;
                }
                else if(!red && blue){
                    state = blueLane;
                    fromRed = 0;
                }
                else if(red && blue && fromRed){
                    state = blueLaneTimed;
                }
                else if(red && blue && !fromRed){
                    state = redLaneTimed;
                }
                numTicks = 0;
            }
            else if(numTicks < 5){
                numTicks++;
            }
        break;

        default:
            state = init;
        break;
    }
}

int main()
{
    // needed to use thread_sleep_for in debugger
    // your board will get stuck without it :(
    #if defined(MBED_DEBUG) && DEVICE_SLEEP
        HAL_DBGMCU_EnableDBGSleepMode();
    #endif

    lights.attach(&checkLights, 1000ms);
    sensors.attach(&checkCars, 100ms);
    while (true){}
}



