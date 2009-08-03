/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_CHANNELFLAGS_WIDGET_H
#define KIS_CHANNELFLAGS_WIDGET_H

#include <QList>
#include <QCheckBox>
#include "krita_export.h"

#include <QScrollArea>
#include <QVector>

class QBitArray;
class KoColorSpace;


/**
 * A simple widget that shows a list of checkboxes in a scrollable
 * area. The checkboxes correspond to the channels in a colorspace and
 * are in the order the channels are packed in the pixel. The
 * resulting QBitArray can be used to decide which channels to modify
 * or not for filters, composition and eveything else.
 */
class KRITAUI_EXPORT KisChannelFlagsWidget : public QScrollArea
{

public:

    KisChannelFlagsWidget(const KoColorSpace * colorSpace, QWidget * parent = 0);
    ~KisChannelFlagsWidget();

public:

    /**
     * Set the channelflags -- they are supposed to be in pixel order.
     */
    void setChannelFlags(const QBitArray & channelFlags);

    /**
     * retrieve the channel flags, in pixel order.
     */
    QBitArray channelFlags() const;

private:

    const KoColorSpace * m_colorSpace;
    QList<QCheckBox*> m_channelChecks;

};

#endif
