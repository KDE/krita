/*
 *  Copyright (c) 2004 Casper Boemann <cbr@boemann.dk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qvaluevector.h>

#include <kdebug.h>

#include <koStore.h>

#include "kis_tileddatamanager.h"
#include "kis_tilediterator.h"
#include "kis_tile.h"
#include "kis_memento.h"

/* The data area is divided into tiles each say 64x64 pixels (defined at compiletime)
 * The tiles are laid out in a matrix that can have negative indexes.
 * The matrix grows automatically if needed (a call for writeacces to a tile outside the current extent)
 *  Even though the matrix has grown it may still not contain tiles at specific positions. They are created on demand
 */

KisTiledDataManager::KisTiledDataManager(Q_UINT32 pixelSize)
{
	m_pixelSize = pixelSize;
	m_defaultTile = new KisTile(pixelSize,0,0);
	m_hashTable = new KisTile * [1024];
	for(int i = 0; i < 1024; i++)
		m_hashTable [i] = 0;
	m_numTiles = 0;
	m_currentMemento = 0;
	m_extentMinX = 0x7FFFFFFF;
	m_extentMinY = 0x7FFFFFFF;
	m_extentMaxX = -(0x7FFFFFFE);
	m_extentMaxY = -(0x7FFFFFFE);
}

KisTiledDataManager::KisTiledDataManager(const KisTiledDataManager & dm)
{
	m_pixelSize = dm.m_pixelSize;
	m_defaultTile = new KisTile(*dm.m_defaultTile, dm.m_defaultTile->getCol(), dm.m_defaultTile->getRow());
	m_hashTable = new KisTile * [1024];
	m_numTiles = 0;
	m_currentMemento = 0;
	m_extentMinX = dm.m_extentMinX;
	m_extentMinY = dm.m_extentMinY;
	m_extentMaxX = dm.m_extentMaxX;
	m_extentMaxY = dm.m_extentMaxY;

	// Deep copy every tile
	for(int i = 0; i < 1024; i++)
	{
		KisTile *tile = dm.m_hashTable[i];

		m_hashTable[i] = 0;

		while(tile)
		{
			KisTile *newtile = new KisTile(*tile, tile->getCol(), tile->getRow());
			newtile->setNext(m_hashTable[i]);
			m_hashTable[i] = newtile;
			tile = tile->getNext();

			m_numTiles++;
		}
	}

}

KisTiledDataManager::~KisTiledDataManager()
{
	// Deep delete every tile
	for(int i = 0; i < 1024; i++)
	{
		KisTile *tile = m_hashTable[i];

		while(tile)
		{
			KisTile *deltile = tile;
			tile = tile->getNext();
			delete deltile;
		}
	}
	delete m_hashTable;
	delete m_defaultTile;
}


Q_UINT32 KisTiledDataManager::xToCol(Q_UINT32 x)
{
	// The hack with 16384 is to avoid negative division which is undefined in C++ and the most
	// common result is not like what is desired.
	// however the hack is not perfect either since for coords lower it gives the wrong result
	return (x + 16384 * KisTile::WIDTH) / KisTile::WIDTH - 16384;
}

Q_UINT32 KisTiledDataManager::yToRow(Q_UINT32 y)
{
	// The hack with 16384 is to avoid negative division which is undefined in C++ and the most
	// common result is not like what is desired.
	// however the hack is not perfect either since for coords lower it gives the wrong result
	return (y + 16384 * KisTile::HEIGHT) / KisTile::HEIGHT - 16384;
}

bool KisTiledDataManager::write(KoStore *store)
{
	char str[80];

	sprintf(str, "%d\n", m_numTiles);
	store->write(str,strlen(str));

	for(int i = 0; i < 1024; i++)
	{
		KisTile *tile = m_hashTable[i];

		while(tile)
		{
			sprintf(str, "%d,%d,%d,%d\n", tile->getCol() * KisTile::WIDTH,
							tile->getRow() * KisTile::HEIGHT,
							KisTile::WIDTH, KisTile::HEIGHT);
			store->write(str,strlen(str));

			store->write((char *)tile->m_data, KisTile::HEIGHT * KisTile::WIDTH * m_pixelSize);

			tile = tile->getNext();
		}
	}

	return true;
}
bool KisTiledDataManager::read(KoStore *store)
{
	char str[80];
	Q_INT32 x,y,w,h;

	QIODevice *stream = store->device();

	stream->readLine(str, 79);

	sscanf(str,"%d",&m_numTiles);

	for(Q_UINT32 i = 0; i < m_numTiles; i++)
	{
		stream->readLine(str, 79);
		sscanf(str,"%d,%d,%d,%d",&x,&y,&w,&h);

		// the following is only correct as long as tile size is not changed
		// The first time we change tilesize the dimensions just read needs to be respected
		// but for now we just assume that tiles are the same size as ever.
		Q_UINT32 row = (y + 16384 * KisTile::HEIGHT) / KisTile::HEIGHT - 16384;
		Q_UINT32 col = (x + 16384 * KisTile::WIDTH) / KisTile::WIDTH - 16384;
		Q_UINT32 tileHash = calcTileHash(col, row);

		KisTile *tile = new KisTile(m_pixelSize, col, row);
		updateExtent(col,row);

		store->read((char *)tile->m_data, KisTile::HEIGHT * KisTile::WIDTH * m_pixelSize);

		tile->setNext(m_hashTable[tileHash]);
		m_hashTable[tileHash] = tile;
	}
	return true;
}

void KisTiledDataManager::extent(Q_INT32 &x, Q_INT32 &y, Q_INT32 &w, Q_INT32 &h) const
{
	x = m_extentMinX;
	y = m_extentMinY;
	w = m_extentMaxX - m_extentMinX + 1;
	h = m_extentMaxY - m_extentMinY + 1;
}


void printRect(const QString & s, const QRect & r)
{
	kdDebug() << "crop: " << s << ": (" << r.x() << "," << r.y() << "," << r.width() << "," << r.height() << ")\n";
}

void KisTiledDataManager::setExtent(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	QRect newRect = QRect(x, y, w, h).normalize();
	printRect("newRect", newRect);
	
	QRect oldRect = QRect(m_extentMinX, m_extentMinY, m_extentMaxX - m_extentMinX + 1, m_extentMaxY - m_extentMinY + 1).normalize();
	printRect("oldRect", oldRect);
	
	// Do nothing if the desired size is bigger than we currently are: that is handled by the autoextending automatically
	if (newRect.contains(oldRect)) return;

	// Loop through all tiles, if a tile is wholly outside the extent, add to the memento, then delete it,
	// if the tile is partially outside the extent, clear the outside pixels to black transparent (XXX: use the
	// default pixel for this when avaiable).
	for(int i = 0; i < 1024; i++)
	{
		KisTile *tile = m_hashTable[i];
		KisTile *previousTile = 0;
		
		while(tile)
		{
			kdDebug() << "Tile: " << tile -> getCol() << ", " << tile -> getRow() << "\n";
			
			QRect tileRect = QRect(tile -> getCol() * KisTile::WIDTH, tile -> getRow() * KisTile::HEIGHT, KisTile::WIDTH, KisTile::HEIGHT);
			printRect("tileRect", tileRect);
			
			if (newRect.contains(tileRect)) {
				// Completely inside, do nothing
				previousTile = tile;
				tile = tile->getNext();
			}
			else {
				Q_UINT32 tileHash = calcTileHash(tileRect.x(), tileRect.y());
				ensureTileMementoed(tileRect.x(), tileRect.y(), tileHash, tile);
			
				if (newRect.intersects(tileRect)) {
					kdDebug() << "Partially inside, clear the non-intersecting bits\n";

					// Create the intersection of the tile and new rect
					QRect intersection = newRect.intersect(tileRect);
					printRect("intersection", intersection);
					intersection.setRect(intersection.x() - tileRect.x(), intersection.y() - tileRect.y(), intersection.width(), intersection.height());

					// This can be done a lot more efficiently, no doubt, by clearing runs of pixels to the left and the right of
					// the intersecting line.
					for (int y = 0; y < KisTile::HEIGHT; ++y) {
						for (int x = 0; x < KisTile::WIDTH; ++x) {
							if (!intersection.contains(x,y)) {
								Q_UINT8 * ptr = tile -> data(x, y);
								memset(ptr, 0, m_pixelSize);
							}
						}
					}
					previousTile = tile;
					tile = tile->getNext();
				}
				else {
					kdDebug() << "Completely outside, delete this tile. It had already been mementoed\n";
 					KisTile *deltile = tile;
 					tile = tile->getNext();
 					
 					if (previousTile)
						previousTile -> setNext(tile);
					else 
						m_hashTable[i] = 0;
 					delete deltile;
// 					memset(tile -> data(0,0), 0, KisTile::HEIGHT * KisTile::WIDTH * m_pixelSize);
// 					previousTile = tile;
// 					tile = tile->getNext();

				}
			}
		}
	}
	
	// Set the extent correctly
	m_extentMinX = x;
	m_extentMinY = y;
	m_extentMaxX = x + w + 1;
	m_extentMaxY = y + h + 1;
}



void KisTiledDataManager::clear(Q_INT32, Q_INT32, Q_INT32, Q_INT32, Q_UINT8)
{
	//CBR_MISSING should be done more efficient, but for now it tests iterators and manager
}

void KisTiledDataManager::clear(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, Q_UINT8 * def)
{
	//CBR_MISSING
	x=y=w=h=*def;
}

void KisTiledDataManager::paste(KisDataManager * data,  Q_INT32 sx, Q_INT32 sy, Q_INT32 dx, Q_INT32 dy,
			 Q_INT32 w, Q_INT32 h)
{
	//CBR_MISSING
	sx=sy=dx=dy=w=h;data=0;
}




Q_UINT32 KisTiledDataManager::calcTileHash(Q_INT32 col, Q_INT32 row)
{
	return ((row << 5) + (col & 0x1F)) & 0x3FF;
}

KisMemento *KisTiledDataManager::getMemento()
{
	m_currentMemento = new KisMemento();
	return m_currentMemento;
}

void KisTiledDataManager::rollback(KisMemento *memento)
{
	// Rollback means restoring all of the tiles in the memento to our hashtable.

	// But first clear the memento redo hashtable.
	// This is nessesary as new changes might have been done since last rollback (automatic filters)
	for(int i = 0; i < 1024; i++)
	{
		KisTile *tile = memento->m_redoHashTable[i];

		while(tile)
		{
			KisTile *deltile = tile;
			tile = tile->getNext();
			delete deltile;
		}
		memento->m_redoHashTable[i]=0;
	}

	// Now on to the real rollback
	for(int i = 0; i < 1024; i++)
	{
		KisTile *tile = memento->m_hashTable[i];

		while(tile)
		{
			// The memento has a tile stored that we need to roll back
			// Now find the corresponding one in our hashtable
			KisTile *curTile = m_hashTable[i];
			KisTile *preTile = 0;
			while(curTile)
			{
				if(curTile->getRow() == tile->getRow() && curTile->getCol() == tile->getCol())
				{
					break;
				}
				preTile = curTile;
				curTile = curTile->getNext();
			}

			// Remove it from our hashtable
			if(preTile)
				preTile->setNext(curTile->getNext());
			else
				m_hashTable[i]= 0;

			// And put it in the redo hashtable of the memento
			curTile->setNext(memento->m_redoHashTable[i]);
			memento->m_redoHashTable[i] = curTile;

			// Put a copy of the memento tile into our hashtable
			curTile = new KisTile(*tile);
			curTile->setNext(m_hashTable[i]);
			m_hashTable[i] = curTile;

			tile = tile->getNext();
		}
	}
}

void KisTiledDataManager::rollforward(KisMemento *memento)
{
	// Rollforward means restoring all of the tiles in the memento's redo to our hashtable.

	for(int i = 0; i < 1024; i++)
	{
		KisTile *tile = memento->m_redoHashTable[i];

		while(tile)
		{
			// The memento has a tile stored that we need to roll forward
			// Now find the corresponding one in our hashtable
			KisTile *curTile = m_hashTable[i];
			KisTile *preTile = 0;
			while(curTile)
			{
				if(curTile->getRow() == tile->getRow() && curTile->getCol() == tile->getCol())
				{
					break;
				}
				preTile = curTile;
				curTile = curTile->getNext();
			}

			// Remove it from our hashtable
			if(preTile)
				preTile->setNext(curTile->getNext());
			else
				m_hashTable[i]= 0;

			// And delete it (it's equal to the one stored in the memento's undo)
			delete curTile;

			// Put a copy of the memento tile into our hashtable
			curTile = new KisTile(*tile);
			curTile->setNext(m_hashTable[i]);
			m_hashTable[i] = curTile;

			tile = tile->getNext();
		}
	}
}

void KisTiledDataManager::ensureTileMementoed(Q_INT32 col, Q_INT32 row, Q_UINT32 tileHash, KisTile *refTile)
{
	// Basically we search for the tile in the current memento, and if it's already there we do nothing, otherwise
	//  we make a copy of the tile and put it in the current memento

	if( ! m_currentMemento)
		return;

	KisTile *tile = m_currentMemento->m_hashTable[tileHash];
	while(tile != 0)
	{
		if(tile->getRow() == row && tile->getCol() == col)
			break;

		tile = tile->getNext();
	}
	if(tile)
		return; // it has allready been stored

	tile = new KisTile(*refTile);
	tile->setNext(m_currentMemento->m_hashTable[tileHash]);
	m_currentMemento->m_hashTable[tileHash] = tile;
	m_currentMemento->m_numTiles++;
}

void KisTiledDataManager::updateExtent(Q_INT32 col, Q_INT32 row)
{
	if(m_extentMinX > col * KisTile::WIDTH)
		m_extentMinX = col * KisTile::WIDTH;
	if(m_extentMaxX < (col+1) * KisTile::WIDTH - 1)
		m_extentMaxX = (col+1) * KisTile::WIDTH - 1;
	if(m_extentMinY > row * KisTile::HEIGHT)
		m_extentMinY = row * KisTile::HEIGHT;
	if(m_extentMaxY < (row+1) * KisTile::HEIGHT - 1)
		m_extentMaxY = (row+1) * KisTile::HEIGHT - 1;
}

KisTile *KisTiledDataManager::getTile(Q_INT32 col, Q_INT32 row, bool writeAccess)
{
	Q_UINT32 tileHash = calcTileHash(col, row);

	// Lookup tile in hash table
	KisTile *tile = m_hashTable[tileHash];
	while(tile != 0)
	{
		if(tile->getRow() == row && tile->getCol() == col)
			break;

		tile = tile->getNext();
	}

	// Might not have been created yet
	if(! tile)
	{
		if(writeAccess)
		{
			// Create a new tile
			tile = new KisTile(*m_defaultTile, col, row);
			tile->setNext(m_hashTable[tileHash]);
			m_hashTable[tileHash] = tile;
			m_numTiles++;
			updateExtent(col, row);
		}
		else
			// If only read access then it's enough to share a default tile
			tile = m_defaultTile;
	}

	if(writeAccess)
		ensureTileMementoed(col, row, tileHash, tile);

	return tile;
}

KisTile *KisTiledDataManager::getOldTile(Q_INT32 col, Q_INT32 row, KisTile *def)
{
	KisTile *tile = 0;
	Q_UINT32 tileHash = calcTileHash(col, row);

	// Lookup tile in hash table of current memento
	if(m_currentMemento)
	{
		tile = m_currentMemento->m_hashTable[tileHash];
		while(tile != 0)
		{
			if(tile->getRow() == row && tile->getCol() == col)
				break;

			tile = tile->getNext();
		}
	}

	if(! tile)
		return def;
	else
		return tile;
}

Q_UINT8* KisTiledDataManager::pixel(Q_INT32 x, Q_INT32 y)
{
	// XXX: Optimize by using the tiles directly
	return readBytes(x, y, 1, 1);
}

void KisTiledDataManager::setPixel(Q_INT32 x, Q_INT32 y, Q_UINT8 * data)
{
	// XXX: Optimize by using the tiles directly
	writeBytes(data, x, y, 1, 1);
}


Q_UINT8 * KisTiledDataManager::readBytes(Q_INT32 x, Q_INT32 y,
					 Q_INT32 w, Q_INT32 h)
{
	// XXX: Optimize by using the tiles directly
 	if (w < 0)
 		w = 0;

 	if (h < 0)
		h = 0;

 	Q_UINT8 * data = new Q_UINT8[w * h * m_pixelSize];
	Q_UINT8 * ptr = data;

 	// XXX: Isn't this a very slow copy?
	for(Q_INT32 y2 = y; y2 < y + h; y2++)
	{
		// XXX; better use rect iterator here?
		KisTiledHLineIterator hiter = KisTiledHLineIterator(this, x, y2, w, false);
		while(! hiter.isDone())
		{
			memcpy(ptr, hiter.rawData(), m_pixelSize);

			ptr += m_pixelSize;
			++hiter;
		}
	}

	return data;

}


void KisTiledDataManager::writeBytes(Q_UINT8 * bytes,
				     Q_INT32 x, Q_INT32 y,
				     Q_INT32 w, Q_INT32 h)
{
	// XXX: Optimize by using the tiles directly

 	// XXX: Is this correct?
	if (w < 0)
		w = 0;

	if (h < 0)
		h = 0;
	Q_UINT8 * ptr = bytes;


	// XXX: Isn't this a very slow copy?
	for(Q_INT32 y2 = y; y2 < y + h; y2++)
	{
		KisTiledHLineIterator hiter = KisTiledHLineIterator(this, x, y2, w, true);
		while(! hiter.isDone())
		{
			memcpy(hiter.rawData(), ptr , m_pixelSize);

			ptr += m_pixelSize;
			++hiter;
		}
	}
}

