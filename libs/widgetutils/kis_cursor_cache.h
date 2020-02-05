/*
 *  Copyright (c) 2016 Michael Abrahams <miabraha@gmail.com>
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

#ifndef KIS_CURSOR_CACHE_H
#define KIS_CURSOR_CACHE_H

#include <QString>
#include <QCursor>
#include <QObject>
#include <QPair>
#include <QPoint>
#include <QHash>

// KisCursorCache implements a global static database of cursors. This allows
// Krita to load cursor data only once.

class KisCursorCache: public QObject
{
    Q_OBJECT
public:
    static KisCursorCache* instance();
    KisCursorCache();

    QCursor selectCursor;
    QCursor pickerPlusCursor;
    QCursor pickerMinusCursor;
    QCursor penCursor;
    QCursor brushCursor;
    QCursor airbrushCursor;
    QCursor eraserCursor;
    QCursor colorChangerCursor;
    QCursor fillerCursor;

    QCursor load(const QString & cursorName, int hotspotX, int hotspotY);

private:

    // Stores cursor hash
    QHash<QString, QPair<QPoint, QCursor>> cursorHash;
    void init();
};

#endif
