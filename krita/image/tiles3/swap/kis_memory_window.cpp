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

#include "kis_debug.h"
#include "kis_memory_window.h"

#include <QDir>

#define SWP_PREFIX "KRITA_SWAP_FILE_XXXXXX"

KisMemoryWindow::KisMemoryWindow(const QString &swapDir, quint64 writeWindowSize)
    : m_readWindowChunk(0,0),
      m_writeWindowChunk(0,0)
{
    if (!swapDir.isEmpty() && QDir::isAbsolutePath(swapDir)) {
        QString swapFileTemplate = swapDir + QDir::separator() + SWP_PREFIX;
        m_file.setFileTemplate(swapFileTemplate);
    }
    else {
        m_file.setPrefix(SWP_PREFIX);
    }

    m_file.setAutoRemove(true);

    m_writeWindowSize = writeWindowSize;
    m_readWindowSize = writeWindowSize / 4;

    m_file.open();

    m_readWindow = 0;
    m_writeWindow = 0;
}

KisMemoryWindow::~KisMemoryWindow()
{
}

quint8* KisMemoryWindow::getReadChunkPtr(const KisChunkData &readChunk)
{
    adjustWindow(readChunk, &m_readWindow, &m_readWindowChunk, m_readWindowSize);
    return m_readWindow + (readChunk.m_begin - m_readWindowChunk.m_begin);
}

quint8* KisMemoryWindow::getWriteChunkPtr(const KisChunkData &writeChunk)
{
    adjustWindow(writeChunk, &m_writeWindow, &m_writeWindowChunk, m_writeWindowSize);
    return m_writeWindow + (writeChunk.m_begin - m_writeWindowChunk.m_begin);
}

void KisMemoryWindow::adjustWindow(const KisChunkData &requestedChunk,
                                   quint8 **window,
                                   KisChunkData *windowChunk,
                                   quint64 windowSize)
{
    if(!(*window) ||
       !(requestedChunk.m_begin >= windowChunk->m_begin &&
         requestedChunk.m_end <= windowChunk->m_end))
    {
        m_file.unmap(*window);

        if(requestedChunk.size() > windowSize) {
            qWarning() <<
                "KisMemoryWindow: the requested chunk is too "
                "big to fit into the mapping! "
                "Adjusting mapping to avoid SIGSEGV...";

            windowSize = requestedChunk.size();
        }

        windowChunk->setChunk(requestedChunk.m_begin, windowSize);

        if(windowChunk->m_end >= (quint64)m_file.size()) {
            // Align by 32 bytes
            quint64 newSize = (windowChunk->m_end + 1 + 32) & (~31ULL);
            m_file.resize(newSize);
        }

#ifdef __GNUC__
#warning "A workaround for http://bugreports.qt.nokia.com/browse/QTBUG-6330"
#else
#pragma WARNING( "A workaround for http://bugreports.qt.nokia.com/browse/QTBUG-6330" )
#endif
        m_file.exists();

        *window = m_file.map(windowChunk->m_begin, windowChunk->size());
    }
}

