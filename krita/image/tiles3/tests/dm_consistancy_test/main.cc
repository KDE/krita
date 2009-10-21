#include <iostream>

#include "kis_tile_data_store.h"
#include "kis_tiled_data_manager.h"


#define PIXEL_SIZE 5
quint8   pixel[PIXEL_SIZE];

using namespace std;

#define indexFromPoint(_point, _width) \
    ((_point).y()*(_width) + (_point).x())

void dm_rw_test(KisTiledDataManager &dm, qint32 numPixels,
                QRect dataRect, quint8* data);

void dm_clear_test(KisTiledDataManager &dm, qint32 numPixels,
                   QRect dataRect, quint8* data);

void dm_clear_part_test(KisTiledDataManager &dm, qint32 numPixels,
                        QRect dataRect, quint8* data);

void dm_extent_test(KisTiledDataManager &dm, qint32 numPixels,
                    QRect dataRect, quint8* data);

void dm_memento_test(KisTiledDataManager &dm, qint32 numPixels,
                     QRect dataRect, quint8* data);

int main()
{
    memset(pixel, 8, PIXEL_SIZE);

    QRect dataRect;
    dataRect.setCoords(60, 60, 130, 130);

    const qint32 numPixels = dataRect.width() * dataRect.height();

    quint8* data = new quint8[numPixels*PIXEL_SIZE];
    qint32 it = 0;

    for (qint32 i = 0; i < numPixels; i++)
        for (qint32 j = 0; j < PIXEL_SIZE; j++) {
            data[it] = (i + j) % 255;
            it++;
        }

    KisTiledDataManager dm(PIXEL_SIZE, pixel);

//    dm_rw_test(dm, numPixels, dataRect, data);
//    dm_clear_test(dm, numPixels, dataRect, data);
//    dm_clear_part_test(dm, numPixels, dataRect, data);
//    dm_extent_test(dm, numPixels, dataRect, data);

    dm_memento_test(dm, numPixels, dataRect, data);


    delete[] data;
    return 0;
}

void dm_memento_test(KisTiledDataManager &dm, qint32 numPixels,
                     QRect dataRect, quint8* data)
{
    quint8 *newData = new quint8[numPixels*PIXEL_SIZE];;
    quint8 *clonedData = new quint8[numPixels*PIXEL_SIZE];;
    memset(clonedData, 8, numPixels*PIXEL_SIZE);

//    dm.clear();

//    dm.commit();


    dm.writeBytes(data, dataRect.left(), dataRect.top(),
                  dataRect.width(), dataRect.height());
    printf("**************** Written some bytes into (60,60)-(130,130) rect ****************\n");
    dm.debugPrintInfo();

    dm.commit();
    printf("****************************** Commited it *************************************\n");
    dm.debugPrintInfo();

    dm.writeBytes(data, 68, 68, 2, 2);
    dm.commit();
    printf("**************** Written some bytes ovet it (68,68)-(69,69)     ****************\n");
    dm.debugPrintInfo();

    dm.rollback();
    dm.rollforward();
    dm.rollback();
    printf("****************************** Reverted back ***********************************\n");
    dm.debugPrintInfo();

    dm.setExtent(64, 64, 64, 64);
    dm.commit();
    printf("**************** Set extent to (64,64)-(127,127). One tile left.****************\n");
    dm.debugPrintInfo();

    dm.rollback();
    dm.rollforward();
    dm.rollback();
    printf("****************************** Reverted back ***********************************\n");
    dm.debugPrintInfo();

    dm.readBytes(newData, dataRect.left(), dataRect.top(),
                 dataRect.width(), dataRect.height());

    dm.clear();


    qint32 cmpResult = memcmp(data, newData, numPixels * PIXEL_SIZE);
    printf("Memento test:\t\t");
    if (!cmpResult)
        printf("Ok\n");
    else
        printf("FAILED\n");

    delete[] clonedData;
    delete[] newData;
}


void dm_extent_test(KisTiledDataManager &dm, qint32 numPixels,
                    QRect dataRect, quint8* data)
{
    //quint8  clearPixel[]={9,8,7,6,5};
    quint8 *newData = new quint8[numPixels*PIXEL_SIZE];;
    quint8 *clonedData = new quint8[numPixels*PIXEL_SIZE];;

    QRect clearRect = dataRect.adjusted(1, 1, -1, -1);

    memset(clonedData, 8, numPixels*PIXEL_SIZE);
    for (qint32 i = clearRect.top(); i <= clearRect.bottom(); i++)
        for (qint32 j = clearRect.left(); j <= clearRect.right(); j++) {
            qint32 idx = indexFromPoint(QPoint(j - dataRect.left(), i - dataRect.top()), dataRect.width());
            memcpy(clonedData + idx*PIXEL_SIZE, data + idx*PIXEL_SIZE,
                   PIXEL_SIZE);
        }


    dm.writeBytes(data, dataRect.left(), dataRect.top(),
                  dataRect.width(), dataRect.height());

    dm.setExtent(clearRect.left(), clearRect.top(),
                 clearRect.width(), clearRect.height());

    dm.readBytes(newData, dataRect.left(), dataRect.top(),
                 dataRect.width(), dataRect.height());

    qint32 cmpResult = memcmp(clonedData, newData, numPixels * PIXEL_SIZE);
    printf("setExtent test:\t\t");
    if (!cmpResult)
        printf("Ok\n");
    else
        printf("FAILED\n");

    delete[] clonedData;
    delete[] newData;
}

void dm_rw_test(KisTiledDataManager &dm, qint32 numPixels,
                QRect dataRect, quint8* data)
{
    quint8 *newData = new quint8[numPixels*PIXEL_SIZE];
    dm.writeBytes(data, dataRect.left(), dataRect.top(),
                  dataRect.width(), dataRect.height());

    dm.readBytes(newData, dataRect.left(), dataRect.top(),
                 dataRect.width(), dataRect.height());

    qint32 cmpResult = memcmp(data, newData, numPixels * PIXEL_SIZE);

    printf("Read/Write test:\t");
    if (!cmpResult)
        printf("Ok\n");
    else
        printf("FAILED\n");

    delete[] newData;
}

void dm_clear_test(KisTiledDataManager &dm, qint32 numPixels,
                   QRect dataRect, quint8* data)
{
    Q_UNUSED(data);

    quint8 *newData = new quint8[numPixels*PIXEL_SIZE];;
    quint8 *clonedData = new quint8[numPixels*PIXEL_SIZE];;
    memset(clonedData, 8, numPixels*PIXEL_SIZE);

    dm.clear();

    dm.readBytes(newData, dataRect.left(), dataRect.top(),
                 dataRect.width(), dataRect.height());

    qint32 cmpResult = memcmp(clonedData, newData, numPixels * PIXEL_SIZE);
    printf("Full clear test:\t");
    if (!cmpResult)
        printf("Ok\n");
    else
        printf("FAILED\n");

    delete[] clonedData;
    delete[] newData;
}


void dm_clear_part_test(KisTiledDataManager &dm, qint32 numPixels,
                        QRect dataRect, quint8* data)
{
    quint8  clearPixel[] = {9, 8, 7, 6, 5};
    quint8 *newData = new quint8[numPixels*PIXEL_SIZE];;
    quint8 *clonedData = new quint8[numPixels*PIXEL_SIZE];;
    memcpy(clonedData, data, numPixels*PIXEL_SIZE);

    QRect clearRect = dataRect.adjusted(1, 1, -1, -1);

    for (qint32 i = clearRect.top(); i <= clearRect.bottom(); i++)
        for (qint32 j = clearRect.left(); j <= clearRect.right(); j++) {
            qint32 idx = indexFromPoint(QPoint(j - dataRect.left(), i - dataRect.top()), dataRect.width());
            memcpy(clonedData + idx*PIXEL_SIZE, clearPixel, PIXEL_SIZE);
        }


    dm.writeBytes(data, dataRect.left(), dataRect.top(),
                  dataRect.width(), dataRect.height());

    dm.clear(clearRect.left(), clearRect.top(),
             clearRect.width(), clearRect.height(), clearPixel);

    dm.readBytes(newData, dataRect.left(), dataRect.top(),
                 dataRect.width(), dataRect.height());

    qint32 cmpResult = memcmp(clonedData, newData, numPixels * PIXEL_SIZE);
    printf("Partly clear test:\t");
    if (!cmpResult)
        printf("Ok\n");
    else
        printf("FAILED\n");

    delete[] clonedData;
    delete[] newData;
}

