/*
 *  SPDX-FileCopyrightText: 2016 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    QCursor samplerPlusCursor;
    QCursor samplerMinusCursor;
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
