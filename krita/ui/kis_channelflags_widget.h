/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#ifndef KIS_CHANNELFLAGS_WIDGET_H
#define KIS_CHANNELFLAGS_WIDGET_H

#include <QList>
#include <QCheckBox>
#include "krita_export.h"

#include <QScrollArea>
#include <Q3ValueVector>

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

    KisChannelFlagsWidget(const KoColorSpace * colorSpace, QWidget * parent = 0 );
    ~KisChannelFlagsWidget();

public:

    /**
     * Set the channelflags -- they are supposed to be in pixel order.
     */
    void setChannelFlags( const QBitArray & channelFlags );

    /**
     * retrieve the channel flags, in pixel order.
     */
    QBitArray channelFlags() const;

private:

    const KoColorSpace * m_colorSpace;
    QList<QCheckBox*> m_channelChecks;

};

#endif
