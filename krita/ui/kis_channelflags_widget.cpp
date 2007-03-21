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

class KisChannelFlagsWidget::Private
{
public:
    QList<QCheckBox*> channelChecks;
};

KisChannelFlagsWidget::KisChannelFlagsWidget(const KoColorSpace * colorSpace, QWidget * parent )
    : QScrollArea( parent )
    , m_d( new Private() )
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
        m_d->channelChecks << bx;
    }

    w->setLayout( vbl );
    setWidget( w );

}

KisChannelFlagsWidget::~KisChannelFlagsWidget()
{
    delete m_d;
}

QBitArray KisChannelFlagsWidget::channelFlags() const
{
    QBitArray ba( m_d->channelChecks.size() );
    int i = 0;
    foreach( QCheckBox * chk, m_d->channelChecks ) {
        ba.setBit( i, ( chk->checkState() == Qt::Checked ) );
        ++i;
    }
    return ba;
}
