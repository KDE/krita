/*
 *  SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TestProfileGeneration.h"

#include "colorprofiles/LcmsColorProfileContainer.h"

#include <QTest>
#include "sdk/tests/testpigment.h"

#include "kis_debug.h"
#include <math.h>



void TestProfileGeneration::testQuantization()
{
    QVector<double> sRGBPrimaries = {0.6400, 0.3300, 1.0,
                                    0.3000, 0.6000, 1.0,
                                    0.1500, 0.0600, 1.0};
    QVector<double> sRGBPrimariesPreQuantized = {0.639998686, 0.330010138, 1.0,
                                                0.300003784, 0.600003357, 1.0,
                                                0.150002046, 0.059997204, 1.0};
    // Figure out how to prequantisize.
}

void TestProfileGeneration::testSRGBProfileGeneration()
{
    // Generate an sRGB profile.
}

void TestProfileGeneration::testTransferFunctions()
{
    /*
     *  Generate every possible transfer function and check their validity.
     */


    QVector<double> inputValues = {0};
    for (int i = 0; i < 50; i++) {
        //inputValues.insert(0, -.01 * (i+1) );
        inputValues.append(    .01 * (i+1) );
    }

    QVector<double> col;
    cmsToneCurve *curve  = cmsBuildGamma(NULL, 1.0); //= LcmsColorProfileContainer::transferFunction(KoColorProfile::TRC_ITU_R_BT_709_5);

    for (double value : inputValues) {
        double cValue = value;

        /*
         * Rec 709
         * for 1  >=  Lc  >=  β
         *  V = α * Lc^0.45 − ( α − 1 )
         * for β  >  Lc  >=  0
         * V = 4.500 * Lc
         */

        if (value > 0.018){
            cValue = 1.099 * powf(value, 0.45) - (.099) ;
        } else if (value < 0.018 && value > 0.0){
            cValue = 4.5 * value;
        } else {
            cValue = 0.0;
        }



        /*
         * double lValue;
        if (cValue > 0.081){
            lValue = powf((.099+cValue)*1/1.099, 1/ 0.45);
        } else {
            lValue = cValue * 1/4.5;
        }
        */
        QVERIFY2(cmsEvalToneCurveFloat(curve, cValue) == value, "Values don't match for rec 709");
    }

    //curve = LcmsColorProfileContainer::transferFunction( KoColorProfile::TRC_IEC_61966_2_1);

    for (double value : inputValues) {
        double cValue = value;

        /*
         * sRGB
         * for 1  >  Lc  >=  β
         *  V = α * Lc( 1÷2.4 ) − ( α − 1 )
         * for β  >  Lc  >=  0
         *  V = 12.92 * Lc
         */

        if (value > 0.0031308){
            cValue = 1.055 * powf(value, 1/2.4) - (.055) ;
        } else if (value < 0.0031308 && value > 0.0){
            cValue = 12.92 * value;
        } else {
            cValue = 0.0;
        }

        /*
        double lValue;
        if (cValue > 0.04045){
            lValue = powf((cValue)*1/1.055 + (.055/1.055), 2.4);
        } else {
            lValue = cValue * 1/12.92;
        }
        */

        QVERIFY2(cmsEvalToneCurveFloat(curve, cValue) == value, "Values don't match for sRGB");
    }

    //curve = LcmsColorProfileContainer::transferFunction(KoColorProfile::TRC_SMPTE_240M);

    for (double value : inputValues) {
        double cValue = value;

        /*
         * SMPTE 240M
         * for 1  >=  Lc  >=  β
         *  V = α * Lc^0.45 − ( α − 1 )
         * for β  >  Lc  >=  0
         * V = 4.0 * Lc
         */

        if (value > 0.0228){
            cValue = 1.1115 * powf(value, 0.45) - (.1115) ;
        } else if (value < 0.0228 && value > 0.0){
            cValue = 4.0 * value;
        } else {
            cValue = 0.0;
        }

        /*
        double lValue;
        if (cValue > 0.0913){
            lValue = powf((.1115+cValue)*1/1.1115, 1/ 0.45);
        } else {
            lValue = cValue * 1/4.0;
        }
        */

        QVERIFY2(cmsEvalToneCurveFloat(curve, cValue) == value, "Values don't match for SMPTE 240M");
    }

    for (double value : inputValues) {
        double cValue = value;

        /* IEC 61966-2-4 ...
         * for Lc between 1 and  β
         *  V = α * Lc^0.45 − ( α − 1)
         * for between β and - β
         *  V = 4.5 * Lc
         * for below -β
         *  V = −α * ( −Lc )^0.45 + ( α − 1)
         *
         * reverse = ((1.0 / 1.099)* V + (-0.099 / 1.099) )^(1.0/0.45) + c
         */

        if (value > 0.018){
            cValue = 1.099 * powf(value, 0.45) - (.099) ;
        } else if (value < 0.018 && value > -0.018){
            cValue = 4.5 * value;
        } else {
            cValue = -1.099 * powf(-value, 0.45) - (-.099) ;
        }


        double lValue;
        if (cValue > 0.018){
            lValue = powf((.099+cValue)*1/1.099, 1/ 0.45);
        } else if (cValue < -0.018) {
             lValue = powf( (cValue/-1.099) + (-.099/-1.099), 1/ 0.45) * -1;
        } else {
            lValue = cValue * 1/4.5;
        }

        // Also not possible in iccv4.

        QVERIFY(lValue==value);
    }

    for (double value : inputValues) {
        double cValue = value;

        /*
         * TRC_ITU_R_BT_1361
         * Extended historical gamut system.
         *
         * for 1.33 > Lc >= β (0.018)
         *  V = α * Lc^0.45 − ( α − 1 )
         * for for β > Lc >= −γ (-0.0045)
         *  V = 4.500 * Lc
         * for −γ >= Lc >= −0.25
         *  V = −( α * ( −4 * Lc )^0.45 − ( α − 1 ) ) ÷ 4
         *
         * reverse:
         * ( (1.0 / 1.099) * Lc + (0.099 / 1.099) ) ^ (1/0.45)
         * Lc* (1/4.5)
         * -(4/1.099)*(Lc*-.25)^(1/0.45)???
         */


        if (value > 0.018){
            cValue = 1.099 * powf(value, 0.45) - (.099) ;
        } else if (value < 0.018 && value > -0.0045){
            cValue = 4.5 * value;
        } else {
            cValue = -(1.099 * powf(-4 * value, 0.45) - (.099)) * 0.25 ;
        }

        double lValue;
        if (cValue > 0.018){
            lValue = powf((.099/1.099)+(cValue*1/1.099), 1/ 0.45);
        } else if (cValue < -0.0045) {
            lValue = powf( (cValue * 4 / -1.099 ) - (0.099 / -1.099), 1/ 0.45) * -.25;
        } else {
            lValue = cValue * 1/4.5;
        }
        // This is not possible in ICC v4.

        QVERIFY(lValue == value);
    }

    //curve = LcmsColorProfileContainer::transferFunction(KoColorProfile::TRC_logarithmic_100);

    for (double value : inputValues) {
        double cValue = value;

        /*
         * Logarithmic 100
         * for Lc between 1 and 0.01:
         *  V = 1.0 + Log10( Lc ) ÷ 2
         * for Lc below 0.01:
         *  V = 0.0
         */

        if (value > 0.01){
            cValue = 1.0+log10(value) *.5 ;
        } else{
            cValue = 0.0;
        }

        /*
        double lValue = powf(10.0, (cValue - 1.0) * 2 );
        if (cValue == 0) {
            lValue = 0;
        }
        */
        QVERIFY2(cmsEvalToneCurveFloat(curve, cValue) == value, "Values don't match for log 100");
    }

    //curve = LcmsColorProfileContainer::transferFunction(KoColorProfile::TRC_logarithmic_100_sqrt10);

    for (double value : inputValues) {
        double cValue = value;

        /*
         * logarithmic_100_sqrt10
         * for Lc between 1 and Sqrt( 10 ) ÷ 1000
         *  V = 1.0 + Log10( Lc ) ÷ 2.5
         * for Lc below Sqrt( 10 ) ÷ 1000
         *  V = 0.0
         */

        if (value > (sqrt(10)/1000)){
            cValue = 1.0+log10(value) * (1/2.5) ;
        } else{
            cValue = 0.0;
        }

        /*

        double lValue = powf(10.0, (cValue - 1.0) * 2.5 );
        if (cValue == 0) {
            lValue = 0;
        }

        */

        QVERIFY2(cmsEvalToneCurveFloat(curve, cValue) == value, "Values don't match for log 100");

    }


    for (double value : inputValues) {
        double cValue = value;
        /*
         * SMPTE_ST_428_1
         * V= (48 * Lo / 52.37)^(1/2.6)
         *
         */

        cValue = powf(48 * value/ 52.37, (1/2.6)) ;


        double lValue = 52.37/48 * powf( cValue , 2.6 );

        //Not possible in icc v4

        QVERIFY(lValue == value);

    }

    for (double value : inputValues) {
        double cValue = value;
        /*
         * HLG
         * for 1  >=  Lc > 1 ÷ 12
         *  V= a * Ln( 12 * Lc − b ) + c
         * for 1 ÷ 12  >=  Lc  >=  0
         *  V = Sqrt( 3 ) * Lc^0.5
         *
         * where...
         * a = 0.17883277
         * b = 0.28466892
         * c = 0.55991073
         *
         */

        double a = 0.17883277;
        double b = 0.28466892;
        double c = 0.55991073;

        if (value> 1/12) {
            cValue = a*log(12*value-b) + c;
        } else {
            cValue = sqrt(3) * powf(value, 0.5);
        }


        double lValue = 52.37/48 * powf( cValue , 2.6 );

        //Not possible in icc v4

        QVERIFY(lValue == value);

    }

}

KISTEST_MAIN(TestProfileGeneration)
