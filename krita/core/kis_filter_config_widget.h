/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
#ifndef _KIS_FILTER_CONFIG_WIDGET_H_
#define _KIS_FILTER_CONFIG_WIDGET_H_

#include <qwidget.h>
#include "kis_filter_configuration.h"

/**
 * Empty base class. Filters can build their own configuration widgets that
 * inherit this class. The configuration widget can emit sigPleaseUpdatePreview
 * when it wants the preview in the filter dialog to be updated. 
 */
class KisFilterConfigWidget : public QWidget {

    Q_OBJECT

public:

    KisFilterConfigWidget(QWidget * parent, const char * name = 0, WFlags f = 0 );
    virtual ~KisFilterConfigWidget();

    virtual void setConfiguration(KisFilterConfiguration * config) = 0;

signals:

    /**
     * Subclasses should emit this signal whenever the preview should be
     * be recalculated.
     */
    void sigPleaseUpdatePreview();
};

#endif
