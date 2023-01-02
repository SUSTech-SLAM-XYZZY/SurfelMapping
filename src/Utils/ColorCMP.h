//
// Created by bill on 1/2/23.
//

#ifndef COLORCMP_H
#define COLORCMP_H

#include <cmath>

extern void convertRGBtoXYZ(int inR, int inG, int inB, double * outX, double * outY, double * outZ);

extern void convertXYZtoLab(double inX, double inY, double inZ, double * outL, double * outa, double * outb);

extern double Lab_color_difference_CIE94( double inL1, double ina1, double  inb1, double inL2, double ina2, double  inb2);

extern double RGB_color_Lab_difference_CIE94( int R1, int G1, int B1, int R2, int G2, int B2);

#endif