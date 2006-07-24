/**
 *  Copyright 2005, 2006 by Gerald Friedland, Kristian Jantz and Lars Knipping
 *
 *  Conversion to C++ for Inkscape by Bob Jamison
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * Note by Bob Jamison:
 * After translating the siox.org Java API to C++ and receiving an
 * education into this wonderful code,  I began again,
 * and started this version using lessons learned.  This version is
 * an attempt to provide an dependency-free SIOX engine that anyone
 * can use in their project with minimal effort.
 *
 * Many thanks to the fine people at siox.org.
 */

//########################################################################
//#  C L A B
//########################################################################

/**
 *
 */

#ifndef CIELAB_H
#define CIELAB_H

class CieLab
{
public:

    /**
     *
     */
    CieLab()
        {
        init();
        C = 0;
        L = A = B = 0.0f;
        }


    /**
     *
     */
    CieLab(unsigned long rgb);


    /**
     *
     */
    CieLab(float lArg, float aArg, float bArg)
        {
        init();
        C = 0;
        L = lArg;
        A = aArg;
        B = bArg;
        }


    /**
     *
     */
    CieLab(const CieLab &other)
        {
        init();
        C = other.C;
        L = other.L;
        A = other.A;
        B = other.B;
        }


    /**
     *
     */
    CieLab &operator=(const CieLab &other)
        {
        init();
        C = other.C;
        L = other.L;
        A = other.A;
        B = other.B;
        return *this;
        }

    /**
     *
     */
    virtual ~CieLab()
        {}

    /**
     * Retrieve a CieLab value via index.
     */
    virtual float operator()(unsigned int index)
        {
        if      (index==0) return L;
        else if (index==1) return A;
        else if (index==2) return B;
        else return 0;
        }


    /**
     *
     */
    virtual void add(const CieLab &other)
        {
        C += other.C;
        L += other.L;
        A += other.A;
        B += other.B;
        }


    /**
     *
     */
    virtual void mul(float scale)
        {
        L *= scale;
        A *= scale;
        B *= scale;
        }


    /**
     *
     */
    virtual unsigned long toRGB();

    /**
     * Approximate cube roots
     */
    double cbrt(double x);

    /**
     *
     */
    double qnrt(double x);

    /**
     * Raise to the 2.4 power
     */
    double pow24(double x);

    /**
     * Squared Euclidian distance between this and another color
     */
    float diffSq(const CieLab &other);

    /**
     * Computes squared euclidian distance in CieLab space for two colors
     * given as RGB values.
     */
    static float diffSq(unsigned int rgb1, unsigned int rgb2);

    /**
     * Computes squared euclidian distance in CieLab space for two colors
     * given as RGB values.
     */
    static float diff(unsigned int rgb0, unsigned int rgb1);


    unsigned int C;
    float L;
    float A;
    float B;

private:

    /**
     *
     */
    void init();


};

#endif //CIELAB_H
