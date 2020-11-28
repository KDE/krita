/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_debug.h"
#include "kis_memory_window.h"

#include <QDir>

#define SWP_PREFIX "KRITA_SWAP_FILE_XXXXXX"

KisMemoryWindow::KisMemoryWindow(const QString &swapDir, quint64 writeWindowSize)
    : m_readWindowEx(writeWindowSize / 4),
      m_writeWindowEx(writeWindowSize)
{
    m_valid = true;

    // swapDir will never be empty, as KisImageConfig::swapDir() always provides
    // us with a (platform specific) default directory, even if none is explicitly
    // configured by the user; also we do not want any logic that determines the
    // default swap dir here.
    KIS_SAFE_ASSERT_RECOVER_NOOP(!swapDir.isEmpty());

    QDir d(swapDir);
    if (!d.exists()) {
        m_valid = d.mkpath(swapDir);
    }

    const QString swapFileTemplate = swapDir + '/' + SWP_PREFIX;

    if (m_valid) {
        m_file.setFileTemplate(swapFileTemplate);
        bool res = m_file.open();
        if (!res || m_file.fileName().isEmpty()) {
            m_valid = false;
        }
    }

    if (!m_valid) {
        qWarning() << "Could not create or open swapfile; disabling swapfile" << swapFileTemplate;
    }
}

KisMemoryWindow::~KisMemoryWindow()
{
}

quint8* KisMemoryWindow::getReadChunkPtr(const KisChunkData &readChunk)
{
    if (!adjustWindow(readChunk, &m_readWindowEx, &m_writeWindowEx)) {
        return nullptr;
    }

    return m_readWindowEx.calculatePointer(readChunk);
}

quint8* KisMemoryWindow::getWriteChunkPtr(const KisChunkData &writeChunk)
{
    if (!adjustWindow(writeChunk, &m_writeWindowEx, &m_readWindowEx)) {
        return nullptr;
    }

    return m_writeWindowEx.calculatePointer(writeChunk);
}

bool KisMemoryWindow::adjustWindow(const KisChunkData &requestedChunk,
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
            warnKrita <<
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

            if (!m_file.resize(newSize)) {
                return false;
            }

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

        if (!adjustingWindow->window) {
            return false;
        }
    }

	return true;
}
