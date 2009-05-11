#include <iostream>

#include "kis_tile.h"
#include "kis_tile_data_store.h"
#include "kis_tile_data.h"


#define PIXEL_SIZE 4
qint8   pixel[PIXEL_SIZE];

using namespace std;

int main()
{
    memset(pixel, 0, PIXEL_SIZE);

    cout << "Starting test\n";

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
    
    
    

}
