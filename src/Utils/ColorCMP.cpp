//
// Created by bill on 1/2/23.
//

#include "ColorCMP.h"

void convertRGBtoXYZ(int inR, int inG, int inB, double * outX, double * outY, double * outZ) {


    double var_R = (inR / 255.0f); //R from 0 to 255
    double var_G = (inG / 255.0f); //G from 0 to 255
    double var_B = (inB / 255.0f); //B from 0 to 255

    if (var_R > 0.04045f)
        var_R = pow(( (var_R + 0.055f) / 1.055f), 2.4f);
    else
        var_R = var_R / 12.92f;

    if (var_G > 0.04045)
        var_G = pow(( (var_G + 0.055f) / 1.055f), 2.4f);
    else
        var_G = var_G / 12.92f;

    if (var_B > 0.04045f)
        var_B = pow(( (var_B + 0.055f) / 1.055f), 2.4f);
    else
        var_B = var_B / 12.92f;

    var_R = var_R * 100;
    var_G = var_G * 100;
    var_B = var_B * 100;

    //Observer. = 2°, Illuminant = D65
    *outX = var_R * 0.4124564 + var_G * 0.3575761 + var_B * 0.1804375;
    *outY = var_R * 0.2126729 + var_G * 0.7151522 + var_B * 0.0721750;
    *outZ = var_R * 0.0193339 + var_G * 0.1191920 + var_B * 0.9503041;
}

void convertXYZtoLab(double inX, double inY, double inZ, double * outL, double * outa, double * outb) {
    // See table above
    double ref_X = 95.047;
    double ref_Y = 100.0;
    double ref_Z = 108.883;

    double var_X = (inX / ref_X); //ref_X = 95.047
    double var_Y = (inY / ref_Y); //ref_Y = 100.0
    double var_Z = (inZ / ref_Z); //ref_Z = 108.883

    const double eps = pow(6.0 / 29.0, 3); // 0.008856
    const double m = 1.0 / 3.0 * pow(eps, -2); //7.787
    const double c = 4.0 / 29.0; // 16.0/116

    if ( var_X > eps )
        var_X = pow(var_X , 1.0/3.0);
    else
        var_X = m * var_X + c;

    if ( var_Y > eps )
        var_Y = pow(var_Y , 1.0/3.0);
    else
        var_Y = m * var_Y  +  c;

    if ( var_Z > eps )
        var_Z = pow(var_Z , 1.0/3.0);
    else
        var_Z = m * var_Z + c ;

    *outL = ( 116.0 * var_Y ) - 16.0;
    *outa = 500.0 * ( var_X - var_Y );
    *outb = 200.0 * ( var_Y - var_Z );
}

double Lab_color_difference_CIE94( double inL1, double ina1, double  inb1, double inL2, double ina2, double  inb2){
    // case Application.GraphicArts:
    double Kl = 1.0;
    double K1 = 0.045;
    double K2 = 0.015;
    // 	break;
    // case Application.Textiles:
    // 	Kl = 2.0;
    // 	K1 = .048;
    // 	K2 = .014;
    // break;

    double deltaL = inL1 - inL2;
    double deltaA = ina1 - ina2;
    double deltaB = inb1 - inb2;

    double c1 = sqrt(pow(ina1, 2) + pow(inb1, 2));
    double c2 = sqrt(pow(ina2, 2) + pow(inb2, 2));
    double deltaC = c1 - c2;

    double deltaH = pow(deltaA,2) + pow(deltaB,2) - pow(deltaC,2);
    deltaH = deltaH < 0 ? 0 : sqrt(deltaH);

    const double sl = 1.f;
    const double kc = 1.f;
    const double kh = 1.f;

    double sc = 1.f + K1*c1;
    double sh = 1.f + K2*c1;

    double i = pow(deltaL/(Kl*sl), 2) +
               pow(deltaC/(kc*sc), 2) +
               pow(deltaH/(kh*sh), 2);

    double finalResult = i < 0 ? 0 : sqrt(i);
    return (finalResult);
}

double RGB_color_Lab_difference_CIE94( int R1, int G1, int B1, int R2, int G2, int B2){
    double x1=0,y1=0,z1=0;
    double x2=0,y2=0,z2=0;
    double l1=0,a1=0,b1=0;
    double l2=0,a2=0,b2=0;

    convertRGBtoXYZ(R1, G1, B1, &x1, &y1, &z1);
    convertRGBtoXYZ(R2, G2, B2, &x2, &y2, &z2);

    convertXYZtoLab(x1, y1, z1, &l1, &a1, &b1);
    convertXYZtoLab(x2, y2, z2, &l2, &a2, &b2);

    return( Lab_color_difference_CIE94(l1 ,a1 ,b1 ,l2 ,a2 ,b2) );
}
