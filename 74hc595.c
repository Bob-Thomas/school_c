#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>

#define   SDI    0   //serial data input
#define   RCLK   1   //memory clock input(STCP)
#define   SRCLK  2   //shift register clock input(SHCP)
#define   REACT_BUTTON 3   //Button

unsigned char LED[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

randr(unsigned int min, unsigned int max) {
       double scaled = (double)rand()/RAND_MAX;

       return (max - min +1)*scaled + min;
}


void pulse(int pin)
{
        digitalWrite(pin, 0);
        digitalWrite(pin, 1);
}

void SIPO(unsigned char byte)
{
        int i;

        for(i=0; i<8; i++) {
                digitalWrite(SDI, ((byte & (0x80 >> i)) > 0));
                pulse(SRCLK);
        }
}

void init(void)
{
        pinMode(SDI, OUTPUT); //make P0 output
        pinMode(RCLK, OUTPUT); //make P0 output
        pinMode(SRCLK, OUTPUT); //make P0 output
        pinMode(REACT_BUTTON, INPUT);
        pullUpDnControl(REACT_BUTTON, PUD_UP);  //pull up to 3.3V,make GPIO1 a stable level

        digitalWrite(SDI, 0);
        digitalWrite(RCLK, 0);
        digitalWrite(SRCLK, 0);
        SIPO(0x00);
        pulse(RCLK);
}

int main(void)
{
    int randomTimer = randr(4, 10);
    int lit = 0;
    int set = 0;
    struct timeb startTime, currentTime, litTimer;
    if(wiringPiSetup() == -1) { //when initialize wiring failed,print messageto screen
            printf("setup wiringPi failed !");
            return 1;
    }

    init();
    ftime(&startTime);
    while(1) {
        //update the currentime timestamp
        ftime(&currentTime);
        //get elapsed seconds since execution of the program
        int elapsed = currentTime.time-startTime.time;
        //get the input value of the button pin
        int value = digitalRead(REACT_BUTTON);

        //If the led is on and the button is pressed
        if(lit == 1 && value == 1) {
          //calculate the reaction speed
          int reactionSpeed = (int) (1000.0 * (currentTime.time - litTimer.time)
          + (currentTime.millitm - litTimer.millitm));
          if(reactionSpeed <= 200) {
              SIPO(LED[3]);
              pulse(RCLK);
          } else if(reactionSpeed <= 300) {
              SIPO(LED[2]);
              pulse(RCLK);
          } else {
              SIPO(LED[1]);
              pulse(RCLK);
          }
          printf("Your reaction time is %ums\n", reactionSpeed);
          return 0;
        }
        //If the elapsed time is greater or equal than the random delay
        if(elapsed >= randomTimer) {
          SIPO(LED[4]);
          pulse(RCLK);
          lit = 1;
        }
        //If the led is lit and the timestamp hasn't been set yet
        if(lit == 1 && set == 0) {
          set = 1;
          ftime(&litTimer);
        }
    }

    return 0;
}
