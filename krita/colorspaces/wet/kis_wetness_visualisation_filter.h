/* 
 * kis_wetness_visualisation_filter.h -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#ifndef _WETNESS_VISUALISATION_FILTER_H
#define _WETNESS_VISUALISATION_FILTER_H

#include <qobject.h>
#include <qtimer.h>
#include <kactionclasses.h>

class KisView;

class WetnessVisualisationFilter : public QObject
{
 Q_OBJECT
public:
    WetnessVisualisationFilter(KisView* view);
    virtual ~WetnessVisualisationFilter() {}
    void setAction(KToggleAction* action);
    // XXX: Figure out a way to match a filter exactly to a colorspace
    virtual ColorSpaceIndependence colorSpaceIndependence() { return FULLY_INDEPENDENT; };
    virtual bool workWith(KisColorSpace* cs) { return (cs->id() == KisID("WET")); };
private slots:
    void slotActivated();
    void slotTimeout();

private:
    KisView * m_view;
    KToggleAction * m_action;
    QTimer m_timer;
};

#endif // _WETNESS_VISUALISATION_FILTER_H
