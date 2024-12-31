#include "mbed.h"
# include "headers/song_def.h"
# include "headers/song.h"
#include "headers/NHD_0216HZ.h"
# include <string>
using namespace std;

# define JOYSEL D2
# define BUTTON_1 D3 
# define BUTTON_2 D4 
# define BUTTON_3 D5 
# define BUTTON_4 D6 

# define RED_LED D7
# define YELLOW_LED D8
# define GREEN_LED D12

# define RED 0b100
# define YELLOW 0b010
# define GREEN 0b001
# define SPEAKER D9

Song allsongs [] ={
    FUR_ELISE,
    CANNON_IN_D,
    MINUET_IN_G_MAJOR,
    TURKISH_MARCH,
    NOCTRUNE_IN_E_FLAT,
    WALTZ_NO2,
    SYMPHONY_NO5,
    EINE_KLEINE_NACHTAMUSIK
};

volatile int song_index = 0;
volatile bool is_playing = false;
volatile bool wait_for_new_song = false;
volatile bool song_started = false;
string lcd_text[2];
volatile int k;
volatile int temp_index;
volatile float Volume;

//Define the threads
Thread t1;
Thread t2;

// Define ticker and timeout
Ticker timer; 
Timeout delay;

//Define the LCD display and the Serial
// CS = D10, MOSI = D11, SCK = D13
NHD_0216HZ lcd(SPI_CS, SPI_MOSI, SPI_SCK);

DigitalIn pause(JOYSEL);
DigitalIn get_input(BUTTON_1);
BusIn  song_number (BUTTON_2,BUTTON_3,BUTTON_4);
BusOut leds(GREEN_LED,YELLOW_LED,RED_LED);
PwmOut speaker(SPEAKER);


//Display the song name on the LCD and the RGB LEDs
void update_lcd_leds_thread(){
	while(1){
        if(song_started == false){
            leds = RED;
        }else if(is_playing == true){
            leds = GREEN;
        }else if(wait_for_new_song == false){
            leds = YELLOW;
        }else if(leds == YELLOW){
            leds = 0;
        }else{
            leds = YELLOW;
        }
       ThisThread::sleep_for(500ms);
	}
}

//Define the ticker ISR
void timer_ISR(){ 
    if (is_playing == true){

        if (k<allsongs[song_index].length){

            //Set the PWM duty cycle to zero, if there is a sound pause
            if(*(allsongs[song_index].note +k) == No) speaker = 0;            
            //Set the PWM period, which determines the note of the sound
            else {
                speaker.period(0.001*(*(allsongs[song_index].note + k)));   
                speaker = Volume;
            }    			
            
            k++;

            //Set the time for the next ticker interrupt
            timer.attach(&timer_ISR, ((*(allsongs[song_index].beat + k)/3)+(allsongs[song_index].tempo/2)));
        }
        else{
            //If the musical piece is finished, start again
            k = 0;                                                                 	
            speaker = 0;
        }
    }
}

void no_response() {
    if(wait_for_new_song == true){
        song_index = temp_index;
        lcd_text[0] = allsongs[song_index].name1;
        lcd_text[1] = allsongs[song_index].name2;
        lcd.clr_lcd();
        lcd.set_cursor(0, 0);
        lcd.printf(lcd_text[0].c_str());
        lcd.set_cursor(0, 1);
        lcd.printf(lcd_text[1].c_str());
        wait_for_new_song = false;
        is_playing = true;
        k = 0;
        timer.attach(&timer_ISR, 100ms);  
    }
}

//Define pause button handler
void pause_button_handler() { 
    if(wait_for_new_song == true){
        no_response();
    }else{
        is_playing = !is_playing;
        speaker = 0;
    }
}

//Define get input handler
void get_input_handler() { 
    lcd_text[0] = "Do you want to ";
    lcd_text[1] = "play song " + to_string(temp_index);
    lcd.clr_lcd();
    lcd.set_cursor(0, 0);
    lcd.printf(lcd_text[0].c_str());
    lcd.set_cursor(0, 1);  
    lcd.printf(lcd_text[1].c_str());  
    delay.attach(&no_response, 5s);
}

void polling_loop(){
	while(1){
        if(pause == 0){
            pause_button_handler();
        }
        if(get_input == 0){
            song_started = true;
            wait_for_new_song = true;
            temp_index = song_number;
            get_input_handler();
        }
	
        ThisThread::sleep_for(250ms);
	}
}

void exampleLCDWrite(){
    lcd_text[0] = "Hello ENGR 029";
    lcd_text[1] = "and Hello World! ";
    lcd.clr_lcd();
    lcd.set_cursor(0, 0);
    lcd.printf(lcd_text[0].c_str());
    lcd.set_cursor(0, 1);  
    lcd.printf(lcd_text[1].c_str());  
}


/*----------------------------------------------------------------------------
 MAIN function
 *----------------------------------------------------------------------------*/

int main(){
    // joystick shield
    pause.mode(PullUp);
    get_input.mode(PullUp);
    song_number.mode(PullUp);

	// Initialise and clear the LCD display
	lcd.init_lcd();
    lcd.clr_lcd();
    exampleLCDWrite();

    Volume = 0.5;
  
    t1.start(update_lcd_leds_thread);
    t2.start(polling_loop);

    printf("Choose a song:\r\n");
    for (int i=0; i<8; i++){
        printf("     %d %s %s %s %s\r\n", i, " - ", allsongs[i].name1.c_str(), " ", allsongs[i].name2.c_str());
    }
    printf("Using the Up, Down, and Left Buttons, insert the song number in binary form\r\n");
    printf("Then press the Right Button and release all the buttons\r\n");

	// Wait for timer interrupt
  while(1){
      //__WFI();
	}
}