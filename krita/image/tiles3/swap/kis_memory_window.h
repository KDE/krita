/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_MEMORY_WINDOW_H
#define __KIS_MEMORY_WINDOW_H

#include "krita_export.h"

#include <QTemporaryFile>
#include "kis_chunk_allocator.h"


#define MiB (1ULL << 20)
#define DEFAULT_WINDOW_SIZE 16*MiB

class KRITAIMAGE_EXPORT KisMemoryWindow
{
public:
    KisMemoryWindow(quint64 writeWindowSize = DEFAULT_WINDOW_SIZE);
    ~KisMemoryWindow();

    quint8* getReadChunkPtr(const KisChunkData &readChunk);
    quint8* getWriteChunkPtr(const KisChunkData &writeChunk);

private:
    void adjustWindow(const KisChunkData &requestedChunk, quint8 **window,
                      KisChunkData *windowChunk, quint64 windowSize);

private:
    QTemporaryFile m_file;
    KisChunkData m_readWindowChunk;
    KisChunkData m_writeWindowChunk;

    quint8 *m_readWindow;
    quint8 *m_writeWindow;

    quint64 m_readWindowSize;
    quint64 m_writeWindowSize;
};

#endif /* __KIS_CHUNK_ALLOCATOR_H */

