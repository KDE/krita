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

#ifndef _KIS_PLUGIN_H_
#define _KIS_PLUGIN_H_

#include <ksharedptr.h>
#include <qwidget.h>
#include "kis_types.h"

class KisTileCommand;

class KisFilterConfiguration {
public:
	KisFilterConfiguration(Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height);
public:
	inline Q_INT32 x() { return m_x; };
	inline Q_INT32 y() { return m_y; };
	inline Q_INT32 width() { return m_width; };
	inline Q_INT32 height() { return m_height; };
private:
	Q_INT32 m_x;
	Q_INT32 m_y;
	Q_INT32 m_width;
	Q_INT32 m_height;
};

class KisFilterConfigurationWidget : public QWidget {
// 	Q_OBJECT
public:
	KisFilterConfigurationWidget ( QWidget * parent = 0, const char * name = 0, WFlags f = 0 );
// public signals:
	/** This signal is emited when a value of the configuration is changed 
		*/
	void valueChanged(KisFilterConfiguration* config);
public:
	/** This function return the configuration
		*/
	virtual KisFilterConfiguration* config();
};

class KisFilter : public QObject, public KShared {
	Q_OBJECT
public:
	KisFilter(const QString& name);
public:
	virtual void process(KisPaintDeviceSP, KisFilterConfiguration*, KisTileCommand* ) =0;
public:
	inline QString name() { return m_name; };
public slots:
	void slotActivated();
protected:
	virtual KisFilterConfigurationWidget* createConfigurationWidget(QWidget* parent);
	/** This function return the default configuration of the filter.
		*/
	virtual KisFilterConfiguration* defaultConfiguration();
private slots:
	/** This signal is emited when a value of the configuration is changed 
		*/
	void refreshPreview(KisFilterConfiguration* config);
private:
	QString m_name;
};

#endif
