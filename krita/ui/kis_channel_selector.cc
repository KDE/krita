/*
 *   Copyright (C) 2006 
 *                                                                         
 *   This program is free software; you can redistribute it and/or modify  
 *   it under the terms of the GNU General Public License as published by  
 *   the Free Software Foundation; either version 2 of the License, or     
 *   (at your option) any later version.                                   
 *                                                                         
 *   This program is distributed in the hope that it will be useful,       
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.                          
 *                                                                         
 *   You should have received a copy of the GNU General Public License     
 *   along with this program; if not, write to the                         
 *   Free Software Foundation, Inc.,                                       
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.          
 */

#include "kis_channel_selector.h"

#include <qtoolbutton.h>

class KisChannelSelectorPrivate {

    KisChannelSelectorPrivate()
        {
        }

    QValueVector<KisChannelInfo* channels> channels;
    QMemArray<bool> channelStates;
    QHBoxLayout * layout;
};

KisChannelSelector(QWidget * parent)
    : super(parent)
{
    d = new KisChannelSelectorPrivate();
    d->layout = new QHBoxLayout(this);
}

~virtual KisChannelSelector()
{
    delete d;
}

void setChannels(const QValueVector<KisChannelInfo *> channels &)
{
    d->channels = channels;

    d->layout->deleteAllItems();
    d->layout->setAutoAdd(true);
    
    d->channelStates->resize(channels->size());

    for (int i = 0; i < channels->size(); ++i) {
        d->channelStates[i] = true;
        QToolButton * b = new QToolButton(this);
        b->setToggleButton(true);
        b->setOn(true);
        b->setTextLabel(channels[i]->abbrev());
        QToolTip::add(b, channels[i]->name());
        connect(b, SIGNAL(toggled(bool)), this, SLOT(slotToggle(bool)));
    }
}

void slotToggle(bool toggle) {
    d->channelStates[i] = toggle;
    emit(sigChannelStatesChanged(d->channelStates);
}


#include "kis_channel_selector.moc"
