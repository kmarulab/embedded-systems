#include "mbed.h"
#include "headers/TEA5767.h"
#include <cstdio>
#include "headers/NHD_0216HZ.h"
#include <string>
#include <cmath>

#define ST_AUTO    0      // Auto mode (toggled by the push button)
#define ST_STEREO  1      // Radio module detected a stereo pilot
#define ST_GO_UP   2      // Encoder being turned clockwise
#define ST_GO_DOWN 3      // Encoder being turned counterclockwise
#define ST_SEARCH  4      // Radio module is perfoming an automatic search

//Encoder
// Push button     ---> D4
// Encoder pin "A" ---> D5
// Encoder pin "B" ---> D6
// #define ENCODER_SW ARDUINO_UNO_D4
// #define ENCODER_A  ARDUINO_UNO_D5
// #define ENCODER_B  ARDUINO_UNO_D6

DigitalIn button1(ARDUINO_UNO_D5);
DigitalIn button2(ARDUINO_UNO_D6);

BusIn memory(ARDUINO_UNO_D3, ARDUINO_UNO_D4, ARDUINO_UNO_D7);

NHD_0216HZ lcd(SPI_CS, SPI_MOSI, SPI_SCK);


// main() runs in its own thread in the OS
using namespace std;


//DigitalIn __next(ARDUINO_UNO_D4);
volatile int index = 0;
TEA5767 Radio;
float radioStations[8] = {90.9,96.5,91.5,100.3,106.9,105.3,93.3,102.7};
string lcd_text[2];
int setMem = 0;
float frequency = 96.5;
float memory1 = 96.5;
float memory2 = 96.5;
float memory3 = 96.5;
uint8_t status = 0; 

// volatile int array_index = 0;  // Initialize the array index variable as volatile

// void increment_array_index() {
//     if (__next.read() == 1) {  // Check if the button is pressed
//         array_index++;  // Increment the array index variable by 1
//         if (array_index >= 8) {  // Check if the array index exceeds the array size
//             array_index = 0;  // Reset the array index to 0
//         }
//         printf("%d", array_index);
//         Radio.set_frequency(radioStations[array_index]);
//     }
// }

void printFreq(){
    string str = to_string(frequency);
    str.erase(str.find('.') + 2, string::npos);
    lcd_text[0] = "FM Radio " + str;
    lcd.clr_lcd();
    lcd.set_cursor(0, 0);
    lcd.printf(lcd_text[0].c_str());
}


void memorySet(){
    setMem = !setMem;
    if(setMem){
        string str = to_string(frequency);
        str.erase(str.find('.') + 2, string::npos);
        lcd_text[0] = "Save Channel ";
        lcd_text[1] = str;
        lcd.clr_lcd();
        lcd.set_cursor(0, 0);
        lcd.printf(lcd_text[0].c_str());
        lcd.set_cursor(0, 1);
        lcd.printf(lcd_text[1].c_str());
        return;
    }else{
        printFreq();
        return;
    }      
    return;
}

void changeChannel(){
    while(!button2.read()){
        wait_us(100000);
        if(frequency < 108) {
            frequency += 0.2;
            Radio.set_frequency(frequency);
            // int newFreq = round(frequency * 10);
            // printf("string = %f\n",newFreq/10);
        }
        printFreq();
    }
    while(!button1.read()){
        wait_us(100000);
        if(frequency > 88) {
            frequency -= 0.2;
            Radio.set_frequency(frequency);
        }        
        printFreq();
    }
    setMem = 0;
}

int main()
{
    InterruptIn memButton(ARDUINO_UNO_D2);

    lcd_text[0] = "Welcome to  ";
    lcd_text[1] = "FM Radio! ";
    lcd.init_lcd();
    lcd.clr_lcd();
    lcd.set_cursor(0, 0);
    lcd.printf(lcd_text[0].c_str());
    lcd.set_cursor(0, 1);  
    lcd.printf(lcd_text[1].c_str());  

    // Ticker ticker;
    // ticker.attach(&increment_array_index,0.01);
    Radio.init();
    Radio.set_frequency(frequency);

    memory.mode(PullUp);
    memButton.mode(PullUp);
    button1.mode(PullUp);
    button2.mode(PullUp);
    memButton.rise(&memorySet);

    while(true){
        // printf("membutton = %d\n",memButton.read());
        if(!button1 || !button2){
            changeChannel();
        }
        unsigned char buf[5];
        int stereo;
        int signalLevel;
        int searchDirection;
        int i;
        i=0;
    
        if(memory != 7){
        wait_us(100000);
        if(setMem){
            if(memory == 6){
                memory1 = frequency;
                setMem = 0;
            }
            if(memory == 5){
                memory2 = frequency;
                setMem = 0;
            }
            if(memory == 3){
                memory3 = frequency;
                setMem = 0;
            }
        }else{
            if(memory == 6){
                Radio.set_frequency(memory1);
                frequency = memory1;
            }
            if(memory == 5){
                Radio.set_frequency(memory2);
                frequency = memory2;
            }
            if(memory == 3){
                Radio.set_frequency(memory3);
                frequency = memory3;
            }
            printFreq();
        }
    }
    // Update the Auto / Manual indicator
    // lcd.setCursor(12, 1);
    // lcd.write(bitRead(status, ST_AUTO) ? 'A' : 'M');
    
    // if bitRead(status, ST_AUTO)   // Auto/manual LED 
    // {
    
    //     digitalWrite(LED, LOW);
    // }
    // else
    // {
        
    //     digitalWrite(LED, HIGH);
    // }

    // printf("here\n");

    // if (Radio.read_status(buf) == 0) {
        // Get radio data
        // frequency = floor(Radio.frequency_available(buf) / 100000 + .5) / 10;
        // stereo = Radio.stereo(buf);
    //     // 0 <= Radio.signal_level <= 15
    //     signalLevel = (Radio.signal_level(buf) * 100) / 15;

    //     analogsignal=map(signalLevel,0,100,0,255);
    //     analogscale=map(frequency,88,114,0,255);
    //     stereoled=map(stereo,0.7,1,0,255);
    //     analogWrite (5,analogsignal);  // Signal meter
    //     analogWrite (6,analogscale);   //frequency meter
    //     analogWrite (7,stereoled);    //stereo LED

    //     // Update the radio dial
    //     updateScale();
        
    //     // Signal level indicator
    //     lcd.setCursor(0, 1);
    //     lcd.write(183);    // Japanese character that looks like an antenna :)
    //     if(signalLevel < 100) lcd.write(' ');
    //     lcd.print(signalLevel);
    //     lcd.write('%');

    //     // Frequency indicator
    //     lcd.setCursor(6, 1);
    //     if(frequency < 100) lcd.write(' ');
    //     lcd.print(frequency, 1);

    //     // Mono / stereo indicator
    //     lcd.setCursor(14, 1);
    //     if(stereo){
    //     lcd.write(STEREO_CHAR_S);
    //     lcd.write(STEREO_CHAR_T);
    //     } else
    //     lcd.print("  ");
    // }
    
    // if(bitRead(status, ST_SEARCH)) {   // Is the radio performing an automatic search?
    //     if(Radio.process_search(buf, searchDirection) == 1) {
    //     bitWrite(status, ST_SEARCH, 0);
    //     }
    // }
    


    //     precrtajScale();  
    // }

    // if(digitalRead(button_freq1)==HIGH){
    //     if(frequency > 92){
    //     frequency=92;
    //     Radio.set_frequency(frequency);
    //     bitWrite(status, ST_GO_DOWN, 0);
    //     }
    //     else
    //     {
    //         frequency=92;
    //         Radio.set_frequency(frequency);
    //         bitWrite(status, ST_GO_UP, 0);
    //     }
    //     precrtajScale();
    // }

    // if(digitalRead(button_freq3)==HIGH){
    //     if(frequency > 97){
    //         frequency=97;
    //         Radio.set_frequency(frequency);
    //         bitWrite(status, ST_GO_DOWN, 0);
    //     }
    //     else{
    //         frequency=97;
    //         Radio.set_frequency(frequency);
    //         bitWrite(status, ST_GO_UP, 0);
    //     }
    //     precrtajScale();
    // }

    // if(digitalRead(button_freq4)==HIGH){
    //     if(frequency > 101.2){
            
    //         frequency=101.2;
    //         Radio.set_frequency(frequency);
    //         bitWrite(status, ST_GO_DOWN, 0);
    //     }
    //     else{
    //         frequency=101.2;
    //         Radio.set_frequency(frequency);
    //         bitWrite(status, ST_GO_UP, 0);
    //     }
    //     precrtajScale();
    // }

    }

// }


    // void precrtajScale() {
    // int i;
    // lcd.clear();
    //     for(i = 0; i < 16; i++)
    //         lcd.write(SCALE_CLEAR);
    // }

    

}