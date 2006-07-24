//########################################################################
//#  C L A B
//########################################################################

#include <map>
#include <math.h>
#include "cielab.h"

static std::map<unsigned long, CieLab> clabLookupTable;

/**
 * Convert integer A, R, G, B values into an pixel value.
 */
static unsigned long getRGB(int a, int r, int g, int b)
{
    if (a<0)  a=0;
    else if (a>255) a=255;

    if (r<0) r=0;
    else if (r>255) r=255;

    if (g<0) g=0;
    else if (g>255) g=255;

    if (b<0) b=0;
    else if (b>255) b=255;

    return (a<<24)|(r<<16)|(g<<8)|b;
}



/**
 * Convert float A, R, G, B values (0.0-1.0) into an pixel value.
 */
static unsigned long getRGB(float a, float r, float g, float b)
{
    return getRGB((int)(a * 256.0),
                  (int)(r * 256.0),
                  (int)(g * 256.0),
                  (int)(b * 256.0));
}



//#########################################
//# Root approximations for large speedup.
//# By njh!
//#########################################
static const int ROOT_TAB_SIZE = 16;
static float cbrt_table[ROOT_TAB_SIZE +1];

double CieLab::cbrt(double x)
{
    double y = cbrt_table[int(x*ROOT_TAB_SIZE )]; // assuming x \in [0, 1]
    y = (2.0 * y + x/(y*y))/3.0;
    y = (2.0 * y + x/(y*y))/3.0; // polish twice
    return y;
}

static float qn_table[ROOT_TAB_SIZE +1];

double CieLab::qnrt(double x)
{
    double y = qn_table[int(x*ROOT_TAB_SIZE )]; // assuming x \in [0, 1]
    double Y = y*y;
    y = (4.0*y + x/(Y*Y))/5.0;
    Y = y*y;
    y = (4.0*y + x/(Y*Y))/5.0; // polish twice
    return y;
}

double CieLab::pow24(double x)
{
    double onetwo = x*qnrt(x);
    return onetwo*onetwo;
}


static bool _clab_inited_ = false;
void CieLab::init()
{
    if (!_clab_inited_)
        {
        cbrt_table[0] = pow(float(1)/float(ROOT_TAB_SIZE*2), 0.3333);
	qn_table[0]   = pow(float(1)/float(ROOT_TAB_SIZE*2), 0.2);
        for(int i = 1; i < ROOT_TAB_SIZE +1; i++)
            {
            cbrt_table[i] = pow(float(i)/float(ROOT_TAB_SIZE), 0.3333);
            qn_table[i] = pow(float(i)/float(ROOT_TAB_SIZE), 0.2);
            }
        _clab_inited_ = true;
        }
}
	


/**
 * Construct this CieLab from a packed-pixel ARGB value
 */
CieLab::CieLab(unsigned long rgb)
{
    init();

    //First try looking up in the cache
    std::map<unsigned long, CieLab>::iterator iter;
    iter = clabLookupTable.find(rgb);
    if (iter != clabLookupTable.end())
        {
        CieLab res = iter->second;
        C = res.C;
        L = res.L;
        A = res.A;
        B = res.B;
        }


    int ir  = (rgb>>16) & 0xff;
    int ig  = (rgb>> 8) & 0xff;
    int ib  = (rgb    ) & 0xff;

    float fr = ((float)ir) / 255.0;
    float fg = ((float)ig) / 255.0;
    float fb = ((float)ib) / 255.0;

    //printf("fr:%f fg:%f fb:%f\n", fr, fg, fb);
    if (fr > 0.04045)
        //fr = (float) pow((fr + 0.055) / 1.055, 2.4);
        fr = (float) pow24((fr + 0.055) / 1.055);
    else
        fr = fr / 12.92;

    if (fg > 0.04045)
        //fg = (float) pow((fg + 0.055) / 1.055, 2.4);
        fg = (float) pow24((fg + 0.055) / 1.055);
    else
        fg = fg / 12.92;

    if (fb > 0.04045)
        //fb = (float) pow((fb + 0.055) / 1.055, 2.4);
        fb = (float) pow24((fb + 0.055) / 1.055);
    else
        fb = fb / 12.92;

    fr = fr * 100.0;
    fg = fg * 100.0;
    fb = fb * 100.0;

    // Use white = D65
    float x = fr * 0.4124 + fg * 0.3576 + fb * 0.1805;
    float y = fr * 0.2126 + fg * 0.7152 + fb * 0.0722;
    float z = fr * 0.0193 + fg * 0.1192 + fb * 0.9505;

    float vx = x /  95.047;
    float vy = y / 100.000;
    float vz = z / 108.883;

    //printf("vx:%f vy:%f vz:%f\n", vx, vy, vz);
    if (vx > 0.008856)
        //vx = (float) pow(vx, 0.3333);
        vx = (float) cbrt(vx);
    else
        vx = (7.787 * vx) + (16.0 / 116.0);

    if (vy > 0.008856)
        //vy = (float) pow(vy, 0.3333);
        vy = (float) cbrt(vy);
    else
        vy = (7.787 * vy) + (16.0 / 116.0);

    if (vz > 0.008856)
        //vz = (float) pow(vz, 0.3333);
        vz = (float) cbrt(vz);
    else
        vz = (7.787 * vz) + (16.0 / 116.0);

    C = 0;
    L = 116.0 * vy - 16.0;
    A = 500.0 * (vx - vy);
    B = 200.0 * (vy - vz);

    // Cache for next time
    clabLookupTable[rgb] = *this;

}



/**
 * Return this CieLab's value a a packed-pixel ARGB value
 */
unsigned long CieLab::toRGB()
{
    float vy = (L + 16.0) / 116.0;
    float vx = A / 500.0 + vy;
    float vz = vy - B / 200.0;

    float vx3 = vx * vx * vx;
    float vy3 = vy * vy * vy;
    float vz3 = vz * vz * vz;

    if (vy3 > 0.008856)
        vy = vy3;
    else
        vy = (vy - 16.0 / 116.0) / 7.787;

    if (vx3 > 0.008856)
        vx = vx3;
    else
        vx = (vx - 16.0 / 116.0) / 7.787;

    if (vz3 > 0.008856)
        vz = vz3;
    else
        vz = (vz - 16.0 / 116.0) / 7.787;

    float x =  95.047 * vx; //use white = D65
    float y = 100.000 * vy;
    float z = 108.883 * vz;

    vx = x / 100.0;
    vy = y / 100.0;
    vz = z / 100.0;

    float vr =(float)(vx *  3.2406 + vy * -1.5372 + vz * -0.4986);
    float vg =(float)(vx * -0.9689 + vy *  1.8758 + vz *  0.0415);
    float vb =(float)(vx *  0.0557 + vy * -0.2040 + vz *  1.0570);

    if (vr > 0.0031308)
        vr = (float)(1.055 * pow(vr, (1.0 / 2.4)) - 0.055);
    else
        vr = 12.92 * vr;

    if (vg > 0.0031308)
        vg = (float)(1.055 * pow(vg, (1.0 / 2.4)) - 0.055);
    else
        vg = 12.92 * vg;

    if (vb > 0.0031308)
        vb = (float)(1.055 * pow(vb, (1.0 / 2.4)) - 0.055);
    else
        vb = 12.92 * vb;

    return getRGB(0.0, vr, vg, vb);
}


/**
 * Squared Euclidian distance between this and another color
 */
float CieLab::diffSq(const CieLab &other)
{
    float sum=0.0;
    sum += (L - other.L) * (L - other.L);
    sum += (A - other.A) * (A - other.A);
    sum += (B - other.B) * (B - other.B);
    return sum;
}

/**
 * Computes squared euclidian distance in CieLab space for two colors
 * given as RGB values.
 */
float CieLab::diffSq(unsigned int rgb1, unsigned int rgb2)
{
    CieLab c1(rgb1);
    CieLab c2(rgb2);
    float euclid = c1.diffSq(c2);
    return euclid;
}


/**
 * Computes squared euclidian distance in CieLab space for two colors
 * given as RGB values.
 */
float CieLab::diff(unsigned int rgb0, unsigned int rgb1)
{
    return (float) sqrt(diffSq(rgb0, rgb1));
}
