#include <iostream>

#include <valgrind/callgrind.h>

#include "kis_tile.h"
#include "kis_tile_data_store.h"
#include "kis_tile_data.h"
#include "kis_tiled_data_manager.h"

#define PIXEL_SIZE 6
quint8   pixel[PIXEL_SIZE];

#define NUM_CYCLES 2
#define NUM_CYCLES_SMALL 2


void testPlanarRW(KisTiledDataManager &dm);
void testRW(KisTiledDataManager &dm);

using namespace std;

#include <sys/time.h>
#include <sys/times.h>

#define prop(fst, snd) ((fst-snd)/snd*100.)

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
    memset(pixel, 8, PIXEL_SIZE);
    KisTiledDataManager dm(PIXEL_SIZE, pixel);

    testRW(dm);
    testPlanarRW(dm);

    return 0;
}

void testRW(KisTiledDataManager &dm)
{
    struct timeval tv;
    struct tms     tms;
    double rstime = 0;
    double ustime = 0;
    double custime = 0;

    QRect dataRect;
    dataRect.setCoords(60, 60, 7110, 7110);

    const qint32 dataSize = dataRect.width() * dataRect.height() * PIXEL_SIZE;

    quint8 *data = new quint8[dataSize];
    for (qint32 i = 0; i < dataRect.height(); i++) {
        memset(data, i % 256, dataRect.width()*PIXEL_SIZE);
    }




    /*BLOCK1**************************************************/
    printf("-----------------------------------\n");
    printf("One loop to warm up COW\n");
    for (int i = 0; i < NUM_CYCLES_SMALL; i++) {
        printf("\tcycle %d", i);
        startMeasure("", tv, tms);

        dm.writeBytes(data, dataRect.left(), dataRect.top(),
                      dataRect.width(), dataRect.height());

        endMeasure(tv, tms, rstime, ustime, custime);
    }
    meanValues(NUM_CYCLES_SMALL, rstime, ustime, custime);


    /*BLOCK2**************************************************/
    /*    printf("-----------------------------------\n");
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
    */

    /*BLOCK3**************************************************/
    printf("-----------------------------------\n");
    printf("Now COW is done. Test actual writing\n");
    printf("KisTileProcessor subsystem (threading is OFF)\n");
    zeroValues(rstime, ustime, custime);
    for (int i = 0; i < NUM_CYCLES; i++) {
        printf("\tcycle %d", i);
        startMeasure("", tv, tms);

        dm.writeBytes(data, dataRect.left(), dataRect.top(),
                      dataRect.width(), dataRect.height());

        endMeasure(tv, tms, rstime, ustime, custime);
    }
    meanValues(NUM_CYCLES, rstime, ustime, custime);

    quint8 *read_data = new quint8[dataSize];
    memset(read_data, 255, dataSize);

    /*BLOCK4**************************************************/
    printf("-----------------------------------\n");
    printf("Read and consistency test\n");
    zeroValues(rstime, ustime, custime);
    for (int i = 0; i < NUM_CYCLES; i++) {
        printf("\tcycle %d", i);
        startMeasure("", tv, tms);

        dm.readBytes(read_data, dataRect.left(), dataRect.top(),
                     dataRect.width(), dataRect.height());

        endMeasure(tv, tms, rstime, ustime, custime);
    }
    meanValues(NUM_CYCLES, rstime, ustime, custime);

    printf("Consistency: ");
    int err = memcmp(data, read_data, dataSize);
    if (!err) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        printf("data and read_data are not the same! (%d)\n", err);
    }

    delete[] read_data;
    delete[] data;
}

void testPlanarRW(KisTiledDataManager &dm)
{
    struct timeval tv;
    struct tms     tms;
    double rstime = 0;
    double ustime = 0;
    double custime = 0;


    QRect dataRect;
    dataRect.setCoords(60, 60, 7110, 7110);
    const qint32 numPixels = dataRect.width() * dataRect.height();

    QVector<quint8*> planes;
    QVector<quint8*> read_planes;
    QVector<qint32>  channelSizes;

    for (qint32 i = 0; i < 3; i++) {
        channelSizes.append(i);
        quint8 *data = new quint8[i*numPixels];
        memset(data, i + 10, i*numPixels);
        planes.append(data);
    }

    printf("Strarting test planar write\n");
    zeroValues(rstime, ustime, custime);
    for (int i = 0; i < NUM_CYCLES; i++) {
        printf("\tcycle %d", i);
        startMeasure("", tv, tms);

        dm.writePlanarBytes(planes, channelSizes,
                            dataRect.left(), dataRect.top(),
                            dataRect.width(), dataRect.height());

        endMeasure(tv, tms, rstime, ustime, custime);
    }
    meanValues(NUM_CYCLES, rstime, ustime, custime);



    printf("---\nStrarting test planar read\n");
    zeroValues(rstime, ustime, custime);
    for (int i = 0; i < NUM_CYCLES; i++) {
        printf("\tcycle %d", i);
        startMeasure("", tv, tms);

        read_planes = dm.readPlanarBytes(channelSizes,
                                         dataRect.left(), dataRect.top(),
                                         dataRect.width(), dataRect.height());

        endMeasure(tv, tms, rstime, ustime, custime);
    }
    meanValues(NUM_CYCLES, rstime, ustime, custime);

    int err;
    int errSum = 0;
    for (qint32 i = 0; i < 3; i++) {
        err = memcmp(planes[i], read_planes[i], i * numPixels);
        err *= err;
        errSum += err;
        delete[] planes[i];
        delete[] read_planes[i];

    }

    printf("Planar consistency: ");
    if (!errSum) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        printf("planar and read_planar are not the same! (%d)\n", errSum);
    }
}


