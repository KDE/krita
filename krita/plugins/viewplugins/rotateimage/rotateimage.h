/*
 * rotateimage.h -- Part of Krita
 *
 * Copyright (c) 2004 Michael Thaler (michael.thaler@physik.tu-muenchen.de)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef ROTATEIMAGE_H
#define ROTATEIMAGE_H

#include <kparts/plugin.h>

class KisView;

class RotateImage : public KParts::Plugin
{
    Q_OBJECT
public:
    RotateImage(QObject *parent, const QStringList &);
    virtual ~RotateImage();

private slots:

    void slotRotateImage();
    void slotRotateImage90();
    void slotRotateImage180();
    void slotRotateImage270();
    void slotRotateLayer();

private:

    KisView * m_view;
    KisPainter * m_painter;

};

#endif // ROTATEIMAGE_H
