// RSLK Self Test via UART

/* This example accompanies the books
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
       ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
       ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers",
       ISBN: 978-1466468863, , Jonathan Valvano, copyright (c) 2017
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2017, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/

#include "msp.h"
#include <stdint.h>
#include <string.h>
#include "..\inc\UART0.h"
#include "..\inc\EUSCIA0.h"
#include "..\inc\FIFO0.h"
#include "..\inc\Clock.h"
//#include "..\inc\SysTick.h"
#include "..\inc\SysTickInts.h"
#include "..\inc\CortexM.h"
#include "..\inc\TimerA1.h"
#include "..\inc\Bump.h"
#include "..\inc\BumpInt.h"
#include "..\inc\LaunchPad.h"
#include "..\inc\Motor.h"
#include "../inc/IRDistance.h"
#include "../inc/ADC14.h"
#include "../inc/LPF.h"
#include "..\inc\Reflectance.h"
#include "../inc/TA3InputCapture.h"
#include "../inc/Tachometer.c"

#define P2_4 (*((volatile uint8_t *)(0x42098070)))
#define P2_3 (*((volatile uint8_t *)(0x4209806C)))
#define P2_2 (*((volatile uint8_t *)(0x42098068)))
#define P2_1 (*((volatile uint8_t *)(0x42098064)))
#define P2_0 (*((volatile uint8_t *)(0x42098060)))

void Pause(void){
  while(LaunchPad_Input()==0);  // wait for touch
  while(LaunchPad_Input());     // wait for release
}
// Linked data structure
struct State {
  uint32_t out;                // 2-bit output
  uint32_t delay;              // time to delay in 1ms
  const struct State *next[4]; // Next if 2-bit input is 0-3
};
typedef const struct State State_t;

#define Center    &fsm[0]
#define Left      &fsm[1]
#define Right     &fsm[2]

State_t fsm[3]={
  {0x03, 50, { Center, Left,   Right,  Center }},  // Center
  {0x02, 50, { Left,  Center, Right,  Center }},  // Left
  {0x01, 50, { Right, Left,   Center, Center }}   // Right
};


State_t *Spt;  // pointer to the current state
uint32_t bump;
uint32_t Input;
uint32_t Output;
int32_t originalLeftSteps = 0;
int32_t originalRightSteps = 0;
int32_t leftSteps;
int32_t rightSteps;
/*Run FSM continuously
1) Output depends on State (LaunchPad LED)
2) Wait depends on State
3) Input (LaunchPad buttons)
4) Next depends on (Input,State)
 */
volatile uint32_t ADCflag;
volatile uint32_t nr,nc,nl;

void SensorRead_ISR(void){  // runs at 2000 Hz
  uint32_t raw17,raw12,raw16;
  P1OUT ^= 0x01;         // profile
  P1OUT ^= 0x01;         // profile
  ADC_In17_12_16(&raw17,&raw12,&raw16);  // sample
  nr = LPF_Calc(raw17);  // right is channel 17 P9.0
  nc = LPF_Calc2(raw12);  // center is channel 12, P4.1
  nl = LPF_Calc3(raw16);  // left is channel 16, P9.1
  ADCflag = 1;           // semaphore
  P1OUT ^= 0x01;         // profile
}
void TurnAngle(uint8_t angle){
    /*Tachometer_Get(uint16_t *leftTach, enum TachDirection *leftDir, int32_t *leftSteps,
    uint16_t *rightTach, enum TachDirection *rightDir, int32_t *rightSteps)*/
    // I AM INSANELY GOATED AT CODING
    //EVERYTHING SCALED BY A 1000

    uint16_t leftTach;
    enum TachDirection leftDir;


    uint16_t rightTach;
    enum TachDirection rightDir;


    // really only the steps is important
    Tachometer_Get(&leftTach, &leftDir, &leftSteps,&rightTach, &rightDir, &rightSteps);
    originalLeftSteps = leftSteps;
    originalRightSteps = rightSteps;
    int32_t wheelCircumference = 7000;
    int32_t baseCircumference = 14500;
    uint8_t currentAngle = 0;
    while(currentAngle < angle){
        Motor_Right(900, 900);
        Clock_Delay1ms(20);
        Motor_Stop();
        Clock_Delay1ms(5);
        Tachometer_Get(&leftTach, &leftDir, &leftSteps,&rightTach, &rightDir, &rightSteps);
        int32_t leftStepDifference = leftSteps - originalLeftSteps;
        int32_t rightStepDifference = rightSteps - originalRightSteps;
        int32_t leftDistance = (leftStepDifference * wheelCircumference) / 360;
        int32_t rightDistance = (rightStepDifference * wheelCircumference) / 360;

        // Calculate the difference in distance travelled by both wheels
        int32_t distanceDifference = leftDistance - rightDistance;

        // Calculate the current angle (in radians or degrees)
        currentAngle = (distanceDifference * 360) / (baseCircumference);

    }
    Motor_Stop();

}

// I need some BS led code now

#define SW1       0x02                  // on the left side of the LaunchPad board
#define SW2       0x10                  // on the right side of the LaunchPad board
#define RED       0x01
#define GREEN     0x02
#define BLUE      0x04

//Initialise GPIO Port1 registers
void Port1_Init(void){
  P1->SEL0 = 0x00;
  P1->SEL1 = 0x00;                        // configure P1.4 and P1.1 as GPIO
  P1->DIR = 0x01;                         // make P1.4 and P1.1 in, P1.0 output
  P1->REN = 0x12;                         // enable pull resistors on P1.4 and P1.1
  P1->OUT = 0x12;                         // P1.4 and P1.1 are pull-up
}

//Read Port1 input data register
uint8_t Port1_Input(void){
  return (P1->IN&0x12);                   // read P1.4,P1.1 inputs
}

//Initialise GPIO Port2 registers
void Port2_Init(void){
  P2->SEL0 = 0x00;
  P2->SEL1 = 0x00;                        // configure P2.2-P2.0 as GPIO
  P2->DS = 0x07;                          // make P2.2-P2.0 high drive strength
  P2->DIR = 0x07;                         // make P2.2-P2.0 out
  P2->OUT = 0x00;                         // all LEDs off
}

//Output data to Port1 GPIO pins by writing to Port1 output data register
void Port1_Output(uint8_t data){        // write all of P1.0 outputs
  P1->OUT = (P1->OUT&0xFE)|data;
}

//Output data to Port2 GPIO pins by writing to Port2 output data register
void Port2_Output(uint8_t data){        // write all of P2 outputs
  P2->OUT = data;
}

int LED(void){ uint8_t status;

  Port1_Init();                         // initialize P1.1 and P1.4 and make them inputs (P1.1 and P1.4 built-in buttons)
                                        // initialize P1.0 as output to red LED
  Port2_Init();                         // initialize P2.2-P2.0 and make them outputs (P2.2-P2.0 built-in LEDs)
  while(1){
    status = Port1_Input();
    switch(status){                 // switches are negative logic on P1.1 and P1.4
      case 0x10:                    // SW1 pressed
        Port2_Output(BLUE);
        Port1_Output(1);
        break;
      case 0x02:                    // SW2 pressed
        Port2_Output(RED);
        Port1_Output(1);
        break;
      case 0x00:                    // both switches pressed
        Port2_Output(BLUE+RED);
        Port1_Output(1);
        break;
      case 0x12:                    // neither switch pressed
        Port2_Output(0);
        Port1_Output(0);
        break;
    }
  }
}







uint32_t COUNTBLACKLINES(void){
    return Reflectance_Number(Reflectance_Read(1000));
}
void CASEFOLLOWBLACK(void){ uint32_t heart=0;
  Clock_Init48MHz();
  LaunchPad_Init();
  Spt = Center;

  while(1){
    Output = Spt->out;
    // set output from FSM


    LaunchPad_Output(Output);     // do output to two motors
    switch(Output){

            case 0x03:
                Motor_Forward(1500,1500);
                Clock_Delay1ms(Spt->delay);
                break;
            case 0x01:
                Motor_Left(1500,1500);
                Clock_Delay1ms(Spt->delay);
                break;
            case 0x02:
                Motor_Right(1500,1500);
                Clock_Delay1ms(Spt->delay);
                break;
            default:
                Motor_Stop();
                break;

        }
    Input = Reflectance_Center(1000);    // read sensors
    Spt = Spt->next[Input];
    // next depends on input and state
    heart = heart^1;
    LaunchPad_LED(heart);         // optional, debugging heartbeat
  }
}
void CASETWO(void){

    Pause();
    uint8_t centerValues;
    int timer = 0;
    while(1){
        Motor_Forward(1500,1500);
        centerValues = Reflectance_Center(1000);
        Clock_Delay1ms(100);
        Motor_Stop();
        if(centerValues > 0){
            Motor_Backward(1500,1500);
            Clock_Delay1ms(1000);
            Motor_Stop();
            if(centerValues >= 2){
                Motor_Left(1500,1500);
                Clock_Delay1ms(1000);
                Motor_Stop();

            }
            else{
                Motor_Right(1500,1500);
                Clock_Delay1ms(1000);
                Motor_Stop();
            }
        }
        timer += 100;
        if(timer > 10000){
            break;
        }
    }
    Motor_Stop();


}



void RSLK_Reset(void){
    DisableInterrupts();

    LaunchPad_Init();
    //Initialise modules used e.g. Reflectance Sensor, Bump Switch, Motor, Tachometer etc
    // ... ...

    EnableInterrupts();
}

// RSLK Self-Test
// Sample program of how the text based menu can be designed.
// Only one entry (RSLK_Reset) is coded in the switch case. Fill up with other menu entries required for Lab5 assessment.
// Init function to various peripherals are commented off.  For reference only. Not the complete list.

int main(void) {
  uint32_t cmd=0xDEAD, menu=0;

  DisableInterrupts();
  Clock_Init48MHz();  // makes SMCLK=12 MHz
  //SysTick_Init(48000,2);  // set up SysTick for 1000 Hz interrupts
  Motor_Init();
  Motor_Stop();
  LaunchPad_Init();
  //Bump_Init();
  //Bumper_Init();
  //IRSensor_Init();
  Tachometer_Init();
  EUSCIA0_Init();     // initialize UART
  EnableInterrupts();

  while(1){                     // Loop forever
      // write this as part of Lab 5
      EUSCIA0_OutChar(CR); EUSCIA0_OutChar(LF);
      EUSCIA0_OutString("RSLK Testing"); EUSCIA0_OutChar(CR); EUSCIA0_OutChar(LF);
      EUSCIA0_OutString("[0] RSLK Reset"); EUSCIA0_OutChar(CR); EUSCIA0_OutChar(LF);
      EUSCIA0_OutString("[1] Motor Test"); EUSCIA0_OutChar(CR); EUSCIA0_OutChar(LF);
      EUSCIA0_OutString("[2] IR Sensor Test"); EUSCIA0_OutChar(CR); EUSCIA0_OutChar(LF);
      EUSCIA0_OutString("[3] Bumper Test"); EUSCIA0_OutChar(CR); EUSCIA0_OutChar(LF);
      EUSCIA0_OutString("[4] Reflectance Sensor Test"); EUSCIA0_OutChar(CR); EUSCIA0_OutChar(LF);
      EUSCIA0_OutString("[5] Tachometer Test"); EUSCIA0_OutChar(CR); EUSCIA0_OutChar(LF);

      EUSCIA0_OutString("CMD: ");
      cmd=EUSCIA0_InUDec();
      EUSCIA0_OutChar(CR); EUSCIA0_OutChar(LF);

      switch(cmd){
          case 0:
              RSLK_Reset();
              menu =1;
              cmd=0xDEAD;
              break;
          case 1:
              CASETWO();
              menu =1;
              cmd=0xDEAD;
              break;
          case 2:
              CASEFOLLOWBLACK();
              menu =1;
              cmd=0xDEAD;
              break;
              // ....
              // ....
          case 3:
              Clock_Delay1ms(500);
              TurnAngle(360);
              Clock_Delay1ms(500);
              TurnAngle(90);
              Clock_Delay1ms(500);
              TurnAngle(90);
              Clock_Delay1ms(500);
              TurnAngle(90);
              Clock_Delay1ms(500);
              menu =1;
              cmd = 0xDEAD;
              break;
          case 4:
              bump = Bump_Read();
              menu =1;
              cmd = 0xDEAD;
              break;

          default:
              menu=1;
              break;
      }

      if(!menu)Clock_Delay1ms(3000);
      else{
          menu=0;
      }

      // ....
      // ....
  }
}

#if 0
//Sample program for using the UART related functions.
int Program5_4(void){
//int main(void){
    // demonstrates features of the EUSCIA0 driver
  char ch;
  char string[20];
  uint32_t n;
  DisableInterrupts();
  Clock_Init48MHz();  // makes SMCLK=12 MHz
  EUSCIA0_Init();     // initialize UART
  EnableInterrupts();
  EUSCIA0_OutString("\nLab 5 Test program for EUSCIA0 driver\n\rEUSCIA0_OutChar examples\n");
  for(ch='A'; ch<='Z'; ch=ch+1){// print the uppercase alphabet
     EUSCIA0_OutChar(ch);
  }
  EUSCIA0_OutChar(LF);
  for(ch='a'; ch<='z'; ch=ch+1){// print the lowercase alphabet
    EUSCIA0_OutChar(ch);
  }
  while(1){
    EUSCIA0_OutString("\n\rInString: ");
    EUSCIA0_InString(string,19); // user enters a string
    EUSCIA0_OutString(" OutString="); EUSCIA0_OutString(string); EUSCIA0_OutChar(LF);

    EUSCIA0_OutString("InUDec: ");   n=EUSCIA0_InUDec();
    EUSCIA0_OutString(" OutUDec=");  EUSCIA0_OutUDec(n); EUSCIA0_OutChar(LF);
    EUSCIA0_OutString(" OutUFix1="); EUSCIA0_OutUFix1(n); EUSCIA0_OutChar(LF);
    EUSCIA0_OutString(" OutUFix2="); EUSCIA0_OutUFix2(n); EUSCIA0_OutChar(LF);

    EUSCIA0_OutString("InUHex: ");   n=EUSCIA0_InUHex();
    EUSCIA0_OutString(" OutUHex=");  EUSCIA0_OutUHex(n); EUSCIA0_OutChar(LF);
  }
}
#endif


