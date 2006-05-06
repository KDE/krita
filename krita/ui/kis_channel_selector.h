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
#ifndef KIS_CHANNEL_SELECTOR_H
#define KIS_CHANNEL_SELECTOR_H

#include <qwidget.h>

class QToolButton;

/**
 * Show a row of toggle buttons for the given array of channel info objects.
 * Emit a QValueVector of bools for enabled/disabled channels. The first bools
 * indicates whether all channels are enabled or disabled.
 */
class KisChannelSelector : public QWidget {
    Q_OBJECT

public:
    KisChannelSelector(QWidget * parent);
    ~virtual KisChannelSelector();

    void setChannels(const QValueVector<KisChannelInfo *> channels &);

signals:

    void sigChannelStatesChanged(const QMemArray<bool>);

private slots:

    void slotToggle(bool);
    
private:

    class KisChannelSelectorPrivate;
    KisChannelSelectorPrivate * d;
};

#endif // KIS_CHANNEL_SELECTOR_H
