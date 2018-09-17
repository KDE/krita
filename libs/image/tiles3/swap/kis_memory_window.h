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

#include <QTemporaryFile>

#include "kis_chunk_allocator.h"


#define DEFAULT_WINDOW_SIZE (16*MiB)

class KRITAIMAGE_EXPORT KisMemoryWindow
{
public:
    /**
     * @param swapDir. If the dir doesn't exist, it'll be created, if it's empty QDir::tempPath will be used.
     */
    KisMemoryWindow(const QString &swapDir, quint64 writeWindowSize = DEFAULT_WINDOW_SIZE);
    ~KisMemoryWindow();

    inline quint8* getReadChunkPtr(KisChunk readChunk) {
        return getReadChunkPtr(readChunk.data());
    }

    inline quint8* getWriteChunkPtr(KisChunk writeChunk) {
        return getWriteChunkPtr(writeChunk.data());
    }

    quint8* getReadChunkPtr(const KisChunkData &readChunk);
    quint8* getWriteChunkPtr(const KisChunkData &writeChunk);

private:
    struct MappingWindow {
        MappingWindow(quint64 _defaultSize)
            : chunk(0,0),
              window(0),
              defaultSize(_defaultSize)
        {
        }

        quint8* calculatePointer(const KisChunkData &other) const {
            return window + other.m_begin - chunk.m_begin;
        }

        KisChunkData chunk;
        quint8 *window;
        const quint64 defaultSize;
    };


private:
    bool adjustWindow(const KisChunkData &requestedChunk,
                      MappingWindow *adjustingWindow,
                      MappingWindow *otherWindow);

private:
    QTemporaryFile m_file;

    bool m_valid;
    MappingWindow m_readWindowEx;
    MappingWindow m_writeWindowEx;
};

#endif /* __KIS_MEMORY_WINDOW_H */

