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
    : m_readWindowEx(writeWindowSize / 4),
      m_writeWindowEx(writeWindowSize)
{
    QString swapFileTemplate = (swapDir.isEmpty() ? QDir::tempPath() : swapDir) + QDir::separator() + SWP_PREFIX;
    if (!QDir::exists(swapDir.isEmpty() ? QDir::tempPath() : swapDir)) {
        QDir::mkpath(swapDir.isEmpty() ? QDir::tempPath() : swapDir);
    }
    m_file.setFileTemplate(swapFileTemplate);
    bool res = m_file.open();
    Q_ASSERT(res);
    Q_ASSERT(!m_file.fileName().isEmpty());
}

KisMemoryWindow::~KisMemoryWindow()
{
}

quint8* KisMemoryWindow::getReadChunkPtr(const KisChunkData &readChunk)
{
    adjustWindow(readChunk, &m_readWindowEx, &m_writeWindowEx);

    return m_readWindowEx.calculatePointer(readChunk);
}

quint8* KisMemoryWindow::getWriteChunkPtr(const KisChunkData &writeChunk)
{
    adjustWindow(writeChunk, &m_writeWindowEx, &m_readWindowEx);

    return m_writeWindowEx.calculatePointer(writeChunk);
}

void KisMemoryWindow::adjustWindow(const KisChunkData &requestedChunk,
                                   MappingWindow *adjustingWindow,
                                   MappingWindow *otherWindow)
{
    if(!(adjustingWindow->window) ||
       !(requestedChunk.m_begin >= adjustingWindow->chunk.m_begin &&
         requestedChunk.m_end <= adjustingWindow->chunk.m_end))
    {
        m_file.unmap(adjustingWindow->window);

        quint64 windowSize = adjustingWindow->defaultSize;
        if(requestedChunk.size() > windowSize) {
            qWarning() <<
                "KisMemoryWindow: the requested chunk is too "
                "big to fit into the mapping! "
                "Adjusting mapping to avoid SIGSEGV...";

            windowSize = requestedChunk.size();
        }

        adjustingWindow->chunk.setChunk(requestedChunk.m_begin, windowSize);

        if(adjustingWindow->chunk.m_end >= (quint64)m_file.size()) {
            // Align by 32 bytes
            quint64 newSize = (adjustingWindow->chunk.m_end + 1 + 32) & (~31ULL);

#ifdef Q_OS_WIN32
            /**
             * Workaround for Qt's "feature"
             *
             * On windows QFSEnginePrivate caches the value of
             * mapHandle which is limited to the size of the file at
             * the moment of its (handle's) creation. That is we will
             * not be able to use it after resizing the file.  The
             * only way to free the handle is to release all the
             * mappings we have. Sad but true.
             */
            if (otherWindow->chunk.size()) {
                m_file.unmap(otherWindow->window);
            }
#else
            Q_UNUSED(otherWindow);
#endif

            m_file.resize(newSize);

#ifdef Q_OS_WIN32
            if (otherWindow->chunk.size()) {
                otherWindow->window = m_file.map(otherWindow->chunk.m_begin,
                                                 otherWindow->chunk.size());
            }
#endif
        }

#ifdef Q_OS_UNIX
        // A workaround for https://bugreports.qt-project.org/browse/QTBUG-6330
        m_file.exists();
#endif

        adjustingWindow->window = m_file.map(adjustingWindow->chunk.m_begin,
                                             adjustingWindow->chunk.size());
    }
}
