/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
     * @param swapDir If the dir doesn't exist, it'll be created, if it's empty QDir::tempPath will be used.
     * @param writeWindowSize write window size.
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

