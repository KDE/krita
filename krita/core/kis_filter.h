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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _KIS_FILTER_H_
#define _KIS_FILTER_H_

#include <qobject.h>
#include <qwidget.h>

#include <ksharedptr.h>

#include "kis_types.h"
#include "kis_view.h"
#include "kis_image.h"
#include "kis_layer.h"

class KisTileCommand;
class KisFilterConfigurationWidget;
class KisPreviewDialog;

class KisFilterConfiguration {
};

class KisFilter : public QObject, public KShared {
	Q_OBJECT
public:
	KisFilter(const QString& name);
public:
	virtual void process(KisPaintDeviceSP, KisFilterConfiguration*, const QRect&, KisTileCommand* ) =0;
public:
	inline const QString name() const { return m_name; };
public slots:
	void slotActivated();
protected:
	virtual KisFilterConfigurationWidget* createConfigurationWidget(QWidget* parent);
	KisFilterConfigurationWidget* configurationWidget();
	/** This function return the configuration of the filter.
		*/
	virtual KisFilterConfiguration* configuration();
	/** This function return the colorspace of the active layout
		*/
	KisStrategyColorSpaceSP colorStrategy();
private slots:
	/** This signal is emited when a value of the configuration is changed 
		*/
	void refreshPreview();
private:
	QString m_name;
	KisFilterConfigurationWidget* m_widget;
	KisPreviewDialog* m_dialog;
};

inline KisFilterConfigurationWidget* KisFilter::configurationWidget()
{
	return m_widget;
}

inline KisStrategyColorSpaceSP KisFilter::colorStrategy()
{
	return KisView::activeView()->currentImg()->activeLayer()->colorStrategy();
}


#endif
