#include <iostream>

#include "kis_tile.h"
#include "kis_tile_data_store.h"
#include "kis_tile_data.h"
#include "kis_tiled_data_manager.h"
#include "kis_tile_processors.h"

#define PIXEL_SIZE 5
quint8   pixel[PIXEL_SIZE];

using namespace std;

void tilePlanarProcessorTest(KisTileHashTable &table);
void tileProcessorTest(KisTileHashTable &table);
void tileHashTableTest(KisTileHashTable &table);
void tileCOWTest();


#define indexFromPoint(_point, _width) \
            ((_point).y()*(_width) + (_point).x())


int main()
{
    memset(pixel, 8, PIXEL_SIZE);


    KisTiledDataManager dm(PIXEL_SIZE, pixel);
    
    QRect dataRect;
//    dataRect.setCoords(60,60,6110,6110);
    dataRect.setCoords(60,60,110,110);
    
    KisTileHashTable table;
    KisTileData *defaultTileData1 = globalTileDataStore.createDefaultTileData(PIXEL_SIZE, pixel);
    table.setDefaultTileData(defaultTileData1);

    tileHashTableTest(table);
    tileProcessorTest(table);
    tilePlanarProcessorTest(table);

    table.clear();
    return 0;
}

void tilePlanarProcessorTest(KisTileHashTable &table)
{
    cout << "Starting KisTilePlanarProcessorTest...\n";
    KisTileSP tile;

    tile = table.getTileLazy(1,1);

    const QRect dataRect(0,0,200,200);

    QVector<qint32> channelSizes;
    channelSizes.resize(3);
    channelSizes[0]=2;
    channelSizes[1]=1;
    channelSizes[2]=2;

    QVector<quint8*> data;
    data.resize(3);
    for(int i=0; i<channelSizes.size(); i++) {
        data[i]=new quint8[dataRect.width()*
                          dataRect.height()*
                          channelSizes[i]];
        memset(data[i], 1+i, 
               dataRect.width()*dataRect.height()*channelSizes[i]);
    }

    QRect workRect;
    workRect.setCoords(121,121,134,134);

    KisTileReadWritePlanarProcessorFactory factory(KisTileReadWritePlanarProcessor::WRITE,
                                                   dataRect,
                                                   channelSizes, data);
    
    KisTileProcessorSP tp = factory.getProcessor(workRect, tile);
    tp->run();
    

/*    KisTileReadWritePlanarProcessor wjob(KisTileReadWritePlanarProcessor::WRITE,
                                   workRect, tile, dataRect,
                                   channelSizes, data);
    wjob.run();
*/

    //tile->debugDumpTile();

    for(int i=0; i<channelSizes.size(); i++) {
        memset(data[i], 255, 
               dataRect.width()*dataRect.height()*channelSizes[i]);
    }

    workRect.adjust(-1,-1,1,1);
    KisTileReadWritePlanarProcessor rjob(KisTileReadWritePlanarProcessor::READ,
                                   workRect, tile, dataRect,
                                   channelSizes, data);
    rjob.run();
   

    workRect.adjust(-1,-1,1,1);

    for(int k=0; k<channelSizes.size(); k++) {
        int channelSize=channelSizes[k];
        for(int i=workRect.top(); i<=workRect.bottom(); i++) {
            for(int j=workRect.left(); j<=workRect.right(); j++) {
              //qint32 tmp=0;
	      //memcpy(&tmp, 
	      //	     (data[k]+indexFromPoint(QPoint(j,i), dataRect.width())*channelSize),
	      //	     channelSize);
	      //printf("%4d ", tmp);
	      printf("%4d ", (char) *(data[k]+indexFromPoint(QPoint(j,i), dataRect.width())*channelSize));
            }
            printf("\n");
        }
        printf("\n---\n");
    }
        
    cout << "  Done.\n";
}

void tileProcessorTest(KisTileHashTable &table)
{
    cout << "Starting KisTileProcessorTest...\n";
    KisTileSP tile;
    tile = table.getTileLazy(1,1);

    const QRect dataRect(0,0,200,200);
    quint8 *data = new quint8[dataRect.width()*
                            dataRect.height()*
                            PIXEL_SIZE];
    
    const qint32 lineSize = dataRect.width()*PIXEL_SIZE;
    quint8 *it = data;
    
    for(int i=dataRect.top(); i<dataRect.bottom(); i++) {
        memset(it, i, lineSize);
        it+=lineSize;
    }


    QRect workRect;
    workRect.setCoords(121,121,134,134);
    KisTileReadWriteProcessor wjob(KisTileReadWriteProcessor::WRITE,
                                   workRect, tile, dataRect, data);
    wjob.run();

    //tile->debugDumpTile();

    memset(data, 255, lineSize*dataRect.height());

    workRect.adjust(-1,-1,1,1);
    KisTileReadWriteProcessor rjob(KisTileReadWriteProcessor::READ,workRect, tile, dataRect, data);
    rjob.run();
   

    workRect.adjust(-1,-1,1,1);
    for(int i=workRect.top(); i<=workRect.bottom(); i++) {
        for(int j=workRect.left(); j<=workRect.right(); j++) {
            printf("%4d ", data[indexFromPoint(QPoint(j,i), dataRect.width())*PIXEL_SIZE]);
        }
        printf("\n");
    }
        
    delete[] data;
    cout << "  Done.\n";
}

void tileHashTableTest(KisTileHashTable &table)
{
    KisTileSP tile;
    cout << "Starting KisTileHashTable test...\n";
    cout << "---\n";
    cout << "  Generating tiles...";
    /**
     * Imagine image of 6400x19200px =)
     */
    for(int i=0; i<100; i++) {
        for(int j=0; j<300; j++) {
            tile = table.getTileLazy(i,j);
        }
        if(!((i+1)%20))
            cout << " " << i+1 << "%";
    }
    cout << "\n" << "  Done.\n";

    table.debugPrintInfo();
    
    cout << "  Deleting some tiles...\n";
    for(int i=50; i<55; i++) {
        for(int j=100; j<105; j++) {
            table.deleteTile(i,j);
        }
    }
    cout << "  Done.\n";
    table.debugPrintInfo();
}

void tileCOWTest()
{
   /*    cout << "Starting test\n";

    KisTileData *defaultTileData = globalTileDataStore.createDefaultTileData(PIXEL_SIZE, pixel);
    cout << "Def. TD created\n";

    KisTile tile(0,0,defaultTileData);
    KisTile tile2(1,1,tile);
    
    cout << "Tile created\n";

    tile2.lockForWrite();
    tile.lockForRead();
    
    KisTile tile4(0,0,tile2);
    
    defaultTileData = globalTileDataStore.createDefaultTileData(PIXEL_SIZE, pixel);
        
    globalTileDataStore.debugPrintList();
    tile.debugPrintInfo();
    tile2.debugPrintInfo();
    tile4.debugPrintInfo();
   */
}



