// IRDistance.c
// Runs on MSP432
// Provide mid-level functions that convert raw ADC
// values from the GP2Y0A21YK0F infrared distance sensors to
// distances in mm.
// Jonathan Valvano
// May 25, 2017

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

// Pololu #3543 Vreg (5V regulator output) connected to all three Pololu #136 GP2Y0A21YK0F Vcc's (+5V) and MSP432 +5V (J3.21)
// Pololu #3543 Vreg (5V regulator output) connected to positive side of three 10 uF capacitors physically near the sensors
// Pololu ground connected to all three Pololu #136 GP2Y0A21YK0F grounds and MSP432 ground (J3.22)
// Pololu ground connected to negative side of all three 10 uF capacitors
// MSP432 P9.0 (J5) (analog input to MSP432) connected to right GP2Y0A21YK0F Vout
// MSP432 P4.1 (J1.5) (analog input to MSP432) connected to center GP2Y0A21YK0F Vout
// MSP432 P9.1 (J5) (analog input to MSP432) connected to left GP2Y0A21YK0F Vout

#include <stdint.h>
#include "../inc/ADC14.h"
#include "msp.h"


/*
 * Routine to convert Filtered Raw ADC values to distance data.
 * Either via curve fitting (hyperbolic, polynomial, log etc), or piece-wise linear method.
 */


int32_t LeftConvert(uint32_t nl) {
    const int N = 6;
    uint32_t n[6] = {15400, 10400, 7500, 6200, 5300, 4200};   // Example n values for left
    uint32_t x[6] = {50, 100, 150, 200, 250, 300};   // Example x values for left

    double A = 0, B = 0;
    double sum_n = 0.0, sum_invx = 0.0, sum_n2 = 0.0, sum_n_invx = 0.0;

        // Compute the sums needed for the least squares method
        int32_t i = 0;
        while(i<N) {
            double invx = 1.0 / x[i]; // Calculate 1/x
            sum_n += n[i];
            sum_invx += invx;
            sum_n2 += n[i] * n[i];
            sum_n_invx += n[i] * invx;
            i++;
        }

        // Least squares formula for the slope (m = 1/A)
        double m = (N * sum_n_invx - sum_n * sum_invx) / (N * sum_n2 - sum_n * sum_n);
        // Intercept formula (c = B/A)
        double c = (sum_invx - m * sum_n) / N;

        // Now calculate A and B
        A = 1.0 / m;  // A = 1/m
        B = c / m;    // B = c/m

    // Compute the left distance
    double left_distance = A / (nl + B);
    uint32_t length = (uint32_t)left_distance;  // Convert to uint32_t (distance in mm)

    return (int32_t)length;  // Return as int32_t (may truncate if needed)
}

// Center distance conversion
int32_t CenterConvert(uint32_t nc) {
    const int N = 6;
    uint32_t n[6] = {9600, 5800, 3600, 2800, 2100, 1600};   // Example n values for center
    uint32_t x[6] = {50, 100, 150, 200, 250, 350};   // Example x values for center

    double A = 0, B = 0;
    double sum_n = 0.0, sum_invx = 0.0, sum_n2 = 0.0, sum_n_invx = 0.0;

        // Compute the sums needed for the least squares method
        int32_t i = 0;
        while(i<N) {
            double invx = 1.0 / x[i]; // Calculate 1/x
            sum_n += n[i];
            sum_invx += invx;
            sum_n2 += n[i] * n[i];
            sum_n_invx += n[i] * invx;
            i++;
        }

        // Least squares formula for the slope (m = 1/A)
        double m = (N * sum_n_invx - sum_n * sum_invx) / (N * sum_n2 - sum_n * sum_n);
        // Intercept formula (c = B/A)
        double c = (sum_invx - m * sum_n) / N;

        // Now calculate A and B
        A = 1.0 / m;  // A = 1/m
        B = c / m;    // B = c/m

    // Compute the center distance
    double center_distance = A / (nc + B);
    uint32_t length = (uint32_t)center_distance;  // Convert to uint32_t (distance in mm)

    return (int32_t)length;  // Return as int32_t
}

// Right distance conversion
int32_t RightConvert(uint32_t nr) {
    const int N = 6;
    uint32_t n[6] = {15400, 10400, 7500, 6200, 5300, 4200};   // Example n values for left
    uint32_t x[6] = {50, 100, 150, 200, 250, 300};   // Example x values for left

    double A = 0, B = 0;
    double sum_n = 0.0, sum_invx = 0.0, sum_n2 = 0.0, sum_n_invx = 0.0;

        // Compute the sums needed for the least squares method
        int32_t i = 0;
        while(i<N) {
            double invx = 1.0 / x[i]; // Calculate 1/x
            sum_n += n[i];
            sum_invx += invx;
            sum_n2 += n[i] * n[i];
            sum_n_invx += n[i] * invx;
            i++;
        }

        // Least squares formula for the slope (m = 1/A)
        double m = (N * sum_n_invx - sum_n * sum_invx) / (N * sum_n2 - sum_n * sum_n);
        // Intercept formula (c = B/A)
        double c = (sum_invx - m * sum_n) / N;

        // Now calculate A and B
        A = 1.0 / m;  // A = 1/m
        B = c / m;    // B = c/m

    // Compute the left distance
    double right_distance = A / (nr + B);
    uint32_t length = (uint32_t)right_distance;  // Convert to uint32_t (distance in mm)

    return (int32_t)length;  // Return as int32_t
}
