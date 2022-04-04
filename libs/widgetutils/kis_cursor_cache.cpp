/*
 *  SPDX-FileCopyrightText: 2016 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_cursor_cache.h"

#include <QScreen>
#include <QWindow>
#include <QBitmap>
#include <QImage>
#include <qmath.h>
#include <QDebug>
#include <QPainter>
#include <QApplication>

#include "KoResourcePaths.h"

Q_GLOBAL_STATIC(KisCursorCache, s_instance)

namespace {

    QCursor loadImpl(const QString &cursorName, int hotspotX, int hotspotY) {
        QPixmap cursorImage = QPixmap(":/" + cursorName);
        if (cursorImage.isNull()) {
            qWarning() << "Could not load cursor from qrc, trying filesystem" << cursorName;

            cursorImage = QPixmap(KoResourcePaths::findResource("kis_pics", cursorName));
            if (cursorImage.isNull()) {
                qWarning() << "Could not load cursor from filesystem" << cursorName;
                return Qt::ArrowCursor;
            }
        }

        return QCursor(cursorImage, hotspotX, hotspotY);
    }

}

KisCursorCache::KisCursorCache() {}

KisCursorCache* KisCursorCache::instance()
{
    return s_instance;
}

QCursor KisCursorCache::load(const QString & cursorName, int hotspotX, int hotspotY)
{
    if (cursorHash.contains(cursorName)) {
        return cursorHash[ cursorName ].second;
    }

    // Otherwise, construct the cursor
    QCursor newCursor = loadImpl(cursorName, hotspotX, hotspotY);
    cursorHash.insert(cursorName, QPair<QPoint, QCursor>(QPoint(hotspotX, hotspotY), newCursor));
    return newCursor;
}
