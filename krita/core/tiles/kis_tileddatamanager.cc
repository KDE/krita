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

#include <koStore.h>

#include "kis_tileddatamanager.h"
#include "kis_tile.h"
#include "kis_iterator.h"
#include "kis_memento.h"

/* The data area is divided into tiles each say 64x64 pixels (defined at compiletime)
 * The tiles are laid out in a matrix that can have negative indexes.
 * The matrix grows automatically if needed (a call for writeacces to a tile outside the current extent)
 *  Even though the matrix has grown it may still not contain tiles at specific positions. They are created on demand
 */
 
KisTiledDataManager::KisTiledDataManager(Q_UINT32 depth)
{
	m_depth = depth;
	m_defaultTile = new KisTile(depth,0,0);
	m_hashTable = new KisTile * [1024];
	for(int i = 0; i < 1024; i++)
		m_hashTable [i] = 0;
	m_numTiles = 0;
	m_currentMemento = 0;
	m_extentMinX = 0;
	m_extentMinY = 0;
	m_extentMaxX = 0;
	m_extentMaxY = 0;
}

KisTiledDataManager::KisTiledDataManager(const KisTiledDataManager & dm)
{
	m_depth = dm.m_depth;
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
			
			store->write((char *)tile->m_data, KisTile::HEIGHT * KisTile::WIDTH * m_depth);
			
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

	for(int i = 0; i < m_numTiles; i++)
	{
		stream->readLine(str, 79);
		sscanf(str,"%d,%d,%d,%d",&x,&y,&w,&h);
		
		// the following is only correct as long as tile size is not changed
		// The first time we change tilesize the dimensions just read needs to be respected
		// but for now we just assume that tiles are the same size as ever.
		Q_UINT32 row = (y + 16384 * KisTile::HEIGHT) / KisTile::HEIGHT - 16384;
		Q_UINT32 col = (x + 16384 * KisTile::WIDTH) / KisTile::WIDTH - 16384;
		Q_UINT32 tileHash = calcTileHash(col, row);
		
		KisTile *tile = new KisTile(m_depth, col, row);
		updateExtent(col,row);
		
		store->read((char *)tile->m_data, KisTile::HEIGHT * KisTile::WIDTH * m_depth);
		
		tile->setNext(m_hashTable[tileHash]);
		m_hashTable[tileHash] = tile;
	}
	return true;
}

Q_UINT32 KisTiledDataManager::size()
{
	return m_depth * KisTile::WIDTH * KisTile::HEIGHT * m_numTiles;
}
 
Q_UINT32 KisTiledDataManager::getDepth()
{
	return m_depth;
}
	
void KisTiledDataManager::extent(Q_INT32 &x, Q_INT32 &y, Q_INT32 &w, Q_INT32 &h) const
{
	x = m_extentMinX;
	y = m_extentMinY;
	w = m_extentMaxX - m_extentMinX + 1;
	h = m_extentMaxY - m_extentMinY + 1;
}
	
void KisTiledDataManager::clear(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, Q_UINT8 def)
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


// ImageBytesVector * KisTiledDataManager::readBytes(Q_INT32 x, Q_INT32 y,
// 						  Q_INT32 w, Q_INT32 h)
// {
// 	if (w < 0)
// 		w = 0;

// 	if (h < 0)
// 		h = 0;

// 	ImageBytesVector * v = new ImageBytesVector (w * h * getDepth());
// 	Q_UINT8 * ptr = v -> pointer();


// 	// XXX: Isn't this a very slow copy?
//         for(Q_INT32 y= y1; y < y1 + h; y++)
//         {
//                 KisHLineIterator hiter = createHLineIterator(x1, w, y, false);
//                 while(! hiter.isDone())
//                 {
//                         memcpy(ptr, (Q_UINT8 *)hiter, getDepth());

//                         ptr += getDepth();
//                         hiter++;
//                 }
//         }

// 	return v;
// }


// void KisTiledDataManager::writeBytes(ImageBytesVector bytes, 
// 				     Q_INT32 x, Q_INT32 y,
// 				     Q_INT32 w, Q_INT32 h,
// 				     Q_UINT8 defaultvalue = 0)
// {
// 	// XXX: Is this correct?
// 	if (w < 0)
// 		w = 0;

// 	if (h < 0)
// 		h = 0;
// 	QUANTUM * ptr = bytes -> pointer();


// 	// XXX: Isn't this a very slow copy?
//         for(Q_INT32 y= y1; y < y1 + h; y++)
//         {
//                 KisHLineIterator hiter = createHLineIterator(x1, w, y, false);
//                 while(! hiter.isDone())
//                 {
//                         memcpy((Q_UINT8 *)hiter, ptr , getDepth());

//                         ptr += getDepth();
//                         hiter++;
//                 }
//         }

// }



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
	// Rollback means restoring all of the tiles in the memento to us.	
	for(int i = 0; i < 1024; i++)
	{
		KisTile *tile = memento->m_hashTable[i];
		
		while(tile)
		{
			// We have a memento-tile, now remove the corresponding one and replace by a copy of the memento-tile
			KisTile *curTile = m_hashTable[i];
			while(curTile)
			{
				if(curTile->getRow() == tile->getRow() && curTile->getCol() == tile->getCol())
				{
				
					break;
				}
				curTile = curTile->getNext();
			}
			// install a copy of the memento tile
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
			// If only read access it is enough to share a default tile
			tile = m_defaultTile;
	}
		
	return tile;
}
