/* This file is part of the KDE project
 *
 * Copyright (C) 2007 Emanuele Tamponi <emanuele@valinor.it>
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef MIXERTOOL_H_
#define MIXERTOOL_H_

#include <KoTool.h>

class KoPointerEvent;
class MixerCanvas;
class QRegion;

class MixerTool : public KoTool
{
    Q_OBJECT

public:
    MixerTool(MixerCanvas *mixer);
    ~MixerTool();

public:
    void setDirty(const QRegion &region);

    // KoTool Implementation.

public slots:

    virtual void activate(bool temporary = false);

    virtual void deactivate();

    virtual void resourceChanged(int key, const QVariant & res);

public:

    /// reimplemented from superclass
    virtual void paint(QPainter &painter, const KoViewConverter &converter);

    /// reimplemented from superclass
    virtual void mousePressEvent(KoPointerEvent *event);

    /// reimplemented from superclass
    virtual void mouseMoveEvent(KoPointerEvent *event);

    /// reimplemented from superclass
    virtual void mouseReleaseEvent(KoPointerEvent *event);

    /// reimplemented from superclass
    virtual void mouseDoubleClickEvent(KoPointerEvent *) {}

protected:
    void initPaint(KoPointerEvent *e);
    void endPaint();

private:

    struct Private;
    Private* const m_d;
};


#endif // MIXERTOOL_H_
