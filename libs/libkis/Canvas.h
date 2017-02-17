/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef LIBKIS_CANVAS_H
#define LIBKIS_CANVAS_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

class KoCanvasBase;

/**
 * Canvas
 */
class KRITALIBKIS_EXPORT Canvas : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Canvas)

public:
    explicit Canvas(KoCanvasBase *canvas, QObject *parent = 0);
    virtual ~Canvas();

public Q_SLOTS:

    qreal zoomLevel() const;
    void setZoomLevel(qreal value);
    void resetZoom();

    void resetRotation();
    qreal rotation() const;
    void setRotation(qreal angle);

    bool mirror() const;
    void setMirror(bool value);

    bool wrapAroundMode() const;
    void setWrapAroundMode(bool enable);

    bool levelOfDetailMode() const;
    void setLevelOfDetailMode(bool enable);

    View *view() const;

private:
    struct Private;
    Private *const d;

};

#endif // LIBKIS_CANVAS_H
