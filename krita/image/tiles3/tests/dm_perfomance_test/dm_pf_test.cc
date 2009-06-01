#include <iostream>

#include "kis_tile.h"
#include "kis_tile_data_store.h"
#include "kis_tile_data.h"
#include "kis_tiled_data_manager.h"
#include "kis_tile_processors.h"

#define PIXEL_SIZE 5
quint8   pixel[PIXEL_SIZE];

using namespace std;

#include <sys/time.h>
#include <sys/times.h>

#define startMeasure(msg, tv, tms)               \
    do {                                         \
        printf("%s:\t", msg);                    \
        gettimeofday(&tv, NULL);                 \
        times(&tms);                             \
    } while(0)

#define endMeasure(tv, tms, rtime, utime, cutime)                    \
    do {                                                             \
        struct timeval new_tv;                                       \
        struct tms     new_tms;                                      \
        gettimeofday(&new_tv, NULL);                                 \
        times(&new_tms);                                             \
        double rsec = double(new_tv.tv_sec - tv.tv_sec) +            \
            double(new_tv.tv_usec - tv.tv_usec)/1000000;             \
        double usec = (double)(new_tms.tms_utime - tms.tms_utime)    \
            / sysconf(_SC_CLK_TCK);                                  \
        double cusec = (double)(new_tms.tms_cutime - tms.tms_cutime) \
            / sysconf(_SC_CLK_TCK);                                  \
        rtime+=rsec;                                                 \
        utime+=usec;                                                 \
        cutime+=cusec;                                               \
        printf("\treal: %5.3f\tuser: %5.3f\tcuser: %5.3f\n",         \
               rsec, usec, cusec);                                   \
    } while(0)
#define meanValues(num, rstime, ustime, custime)                  \
    do {                                                          \
        rstime/=num;                                              \
        ustime/=num;                                              \
        custime/=num;                                             \
        printf("---\n");                                          \
        printf("Mean value:\t\t\treal: %5.3f\tuser: %5.3f"        \
               "\tcuser: %5.3f\n",                                \
               double(rstime), double(ustime), double(custime));  \
    } while(0)

#define zeroValues(rstime, ustime, custime) (rstime=ustime=custime=0)

int main()
{
    struct timeval tv;
    struct tms     tms;
    double rstime=0;
    double ustime=0;
    double custime=0;

    memset(pixel, 8, PIXEL_SIZE);

    KisTiledDataManager dm(PIXEL_SIZE, pixel);
    
    QRect dataRect;
    dataRect.setCoords(60,60,7110,7110);
//    dataRect.setCoords(60,60,110,110);
   
    const qint32 dataSize = dataRect.width()*dataRect.height()*PIXEL_SIZE;

    quint8 *data = new quint8[dataSize];
    memset(data, 1, dataSize);

    
    #define NUM_CYCLES 5
    #define NUM_CYCLES_SMALL 2

    /*BLOCK1**************************************************/
    printf("-----------------------------------\n");
    printf("One loop to warm up ld\n");
    for(int i=0; i<NUM_CYCLES_SMALL; i++) {
        printf("\tcycle %d", i);
        startMeasure("", tv, tms);
        
        dm.writeBytes(data, dataRect.left(), dataRect.top(), 
                      dataRect.width(), dataRect.height());
         
        endMeasure(tv,tms, rstime, ustime, custime);
    }
    meanValues(NUM_CYCLES_SMALL, rstime, ustime, custime);


    /*BLOCK2**************************************************/
    printf("-----------------------------------\n");
    printf("Test writing with heavy COW'ing\n");
    zeroValues(rstime, ustime, custime);
    for(int i=0; i<NUM_CYCLES_SMALL; i++) {
        printf("\tcycle %d", i);
        startMeasure("", tv, tms);

        {
            KisTiledDataManager dm1(PIXEL_SIZE, pixel);
            dm1.writeBytes(data, dataRect.left(), dataRect.top(), 
                           dataRect.width(), dataRect.height());
        }
         
        endMeasure(tv,tms, rstime, ustime, custime);
    }
    meanValues(NUM_CYCLES_SMALL, rstime, ustime, custime);


    /*BLOCK3**************************************************/
    printf("-----------------------------------\n");
    printf("Now COW is done. Test actual writing\n");
    printf("KisTileProcessor subsystem (threading is OFF)\n");
    debugDMFlags=DEBUG_THREADING_OFF;
    zeroValues(rstime, ustime, custime);
    for(int i=0; i<NUM_CYCLES; i++) {
        printf("\tcycle %d", i);
        startMeasure("", tv, tms);

        dm.writeBytes(data, dataRect.left(), dataRect.top(), 
                       dataRect.width(), dataRect.height());
         
        endMeasure(tv,tms, rstime, ustime, custime);
    }
    meanValues(NUM_CYCLES, rstime, ustime, custime);
    double proc_nth = rstime;


    /*BLOCK4**************************************************/
    printf("-----------------------------------\n");
    printf("KisTileProcessor subsystem (threading is ON)\n");
    debugDMFlags=DEBUG_THREADING_ON;
    zeroValues(rstime, ustime, custime);
    for(int i=0; i<NUM_CYCLES; i++) {
        printf("\tcycle %d", i);
        startMeasure("", tv, tms);

        dm.writeBytes(data, dataRect.left(), dataRect.top(), 
                       dataRect.width(), dataRect.height());
         
        endMeasure(tv,tms, rstime, ustime, custime);
    }
    meanValues(NUM_CYCLES, rstime, ustime, custime);
    double proc_th = rstime;

    /*BLOCK5**************************************************/
    printf("-----------------------------------\n");
    printf("Old datamanager subsystem (threading is OFF of course)\n");
//    debugDMFlags=DEBUG_THREADING_OFF;
    zeroValues(rstime, ustime, custime);
    for(int i=0; i<NUM_CYCLES; i++) {
        printf("\tcycle %d", i);
        startMeasure("", tv, tms);

        dm.writeBytesOld(data, dataRect.left(), dataRect.top(), 
                         dataRect.width(), dataRect.height());
         
        endMeasure(tv,tms, rstime, ustime, custime);
    }
    meanValues(NUM_CYCLES, rstime, ustime, custime);
    double old_nth = rstime;

    printf("====================================================\n");
    printf("Comparison \t\t\t(looses, %%)\n");

#define prop(fst, snd) ((fst-snd)/snd*100.)

    printf("threaded vs non-threaded:\t%9.3f%%\n", prop(proc_th, proc_nth));
    printf("threaded vs old:\t\t%9.3f%%\n", prop(proc_th, old_nth));
    printf("non-threaded vs old:\t\t%9.3f%%\n", prop(proc_nth, old_nth));

    delete[] data;
    return 0;
}


