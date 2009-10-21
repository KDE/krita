#include <iostream>

#include "kis_tile.h"
#include "kis_tile_data_store.h"
#include "kis_tile_data.h"
//#include "kis_tiled_data_manager.h"
#include "kis_tile_hash_table.h"

#define PIXEL_SIZE 5
quint8   pixel[PIXEL_SIZE];

using namespace std;

void tileHashTableTest(KisTileHashTable &table);
void tileCOWTest();


#define indexFromPoint(_point, _width) \
    ((_point).y()*(_width) + (_point).x())


int main()
{
    memset(pixel, 8, PIXEL_SIZE);

    QRect dataRect;
    dataRect.setCoords(60, 60, 110, 110);

    KisTileHashTable table;
    KisTileData *defaultTileData1 = globalTileDataStore.createDefaultTileData(PIXEL_SIZE, pixel);
    table.setDefaultTileData(defaultTileData1);

    tileHashTableTest(table);
    table.clear();

    return 0;
}

void tileHashTableTest(KisTileHashTable &table)
{
    KisTileSP tile;
    bool newTile;
    cout << "Starting KisTileHashTable test...\n";
    cout << "---\n";
    cout << "  Generating tiles...";
    /**
     * Imagine image of 6400x19200px =)
     */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 300; j++) {
            tile = table.getTileLazy(i, j, newTile);
        }
        if (!((i + 1) % 20))
            cout << " " << i + 1 << "%";
    }
    cout << "\n" << "  Done.\n";

    table.debugPrintInfo();

    cout << "  Deleting some tiles...\n";
    for (int i = 50; i < 55; i++) {
        for (int j = 100; j < 105; j++) {
            table.deleteTile(i, j);
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



