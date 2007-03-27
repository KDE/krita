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

#include "kis_channelflags_widget.h"
#include <QBitArray>
#include <QScrollArea>
#include <QWidget>
#include <QCheckBox>
#include <QVBoxLayout>

#include <klocale.h>

#include <KoChannelInfo.h>
#include <KoColorSpace.h>


KisChannelFlagsWidget::KisChannelFlagsWidget(const KoColorSpace * colorSpace, QWidget * parent )
    : QScrollArea( parent )
    , m_colorSpace( colorSpace )
{

    setToolTip( "Check the active channels in this layer. Only these channels will be affected by any operation." );
    QWidget * w = new QWidget();
    setBackgroundRole(QPalette::Window);
    QVBoxLayout * vbl = new QVBoxLayout();

    for ( int i = 0; i < colorSpace->channels().size(); ++i ) {
        KoChannelInfo * channel = colorSpace->channels().at( i );
        kDebug() << "Channel: " << channel->name() << endl;
        QCheckBox * bx = new QCheckBox(channel->name(), w );
        bx->setCheckState( Qt::Checked );
        vbl->addWidget( bx );
        m_channelChecks.append( bx );
    }

    w->setLayout( vbl );
    setWidget( w );

}

KisChannelFlagsWidget::~KisChannelFlagsWidget()
{
}

void KisChannelFlagsWidget::setChannelFlags( const QBitArray & cf )
{
    if ( cf.isEmpty() ) return;

    QBitArray channelFlags = m_colorSpace->setChannelFlagsToColorSpaceOrder( cf );
    for ( int i = 0; i < qMin( m_channelChecks.size(), channelFlags.size() ); ++i ) {
        kDebug() << i << ", " << m_channelChecks.at( i )->text() << ", " << channelFlags.testBit( i );
        m_channelChecks.at( i )->setChecked( channelFlags.testBit( i ) );
    }
}

QBitArray KisChannelFlagsWidget::channelFlags() const
{
    bool allTrue = true;
    QBitArray ba( m_channelChecks.size() );

    for ( int i = 0; i < m_channelChecks.size(); ++i ) {
        bool flag = m_channelChecks.at( i )->isChecked();
        if ( !flag ) allTrue = false;
        ba.setBit( i, flag );
    }
    if ( allTrue )
        return QBitArray();
    else
        return m_colorSpace->setChannelFlagsToPixelOrder( ba );
}
