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
#include "kis_filter_registry.h"

class KisTileCommand;
class KisFilterConfigurationWidget;
class KisPreviewDialog;


template<class F>
KisFilterSP createFilter(KisView* view)
{
	KisFilterSP kfi;
	if( view->filterRegistry()->exist( F::name() ) )
	{
		kfi = view->filterRegistry()->get( F::name() );
	} else {
		kfi = new F(view);
		view->filterRegistry()->add(kfi);
	}
	return kfi;
}

class KisFilterConfiguration {
};

class KisFilter : public QObject, public KShared {
	Q_OBJECT
public:
	KisFilter(const QString& name, KisView * view);
	virtual ~KisFilter() {}

public:
	virtual void process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration*, const QRect&, KisTileCommand* ) = 0;
	void process(KisPaintDeviceSP, KisFilterConfiguration*, const QRect&, KisTileCommand* );
	
public:
	virtual KisFilterConfiguration* configuration(KisFilterConfigurationWidget*);
	inline const QString name() const { return m_name; };
	virtual KisFilterConfigurationWidget* createConfigurationWidget(QWidget* parent);

// XXX: Why is this commented out?
// 	KisFilterConfigurationWidget* configurationWidget(QWidget* parent);
	inline KisView* view();
public slots:

	void slotActivated();

protected:
// XXX: Why is this commented out?
// 	KisFilterConfigurationWidget* configurationWidget();

  	KisStrategyColorSpaceSP colorStrategy();

private slots:

	void refreshPreview();

private:
	QString m_name;
	KisView * m_view;
	KisFilterConfigurationWidget* m_widget;
	KisPreviewDialog* m_dialog;
};

inline KisView* KisFilter::view()
{
	return m_view;
}


inline KisStrategyColorSpaceSP KisFilter::colorStrategy()
{
	if (!m_view) return 0;
	KisImageSP img = m_view -> currentImg();

	if (!img) return 0;
	KisLayerSP layer = img -> activeLayer();

	if (!layer) return 0;
	return layer -> colorStrategy();
}

inline void KisFilter::process(KisPaintDeviceSP dev, KisFilterConfiguration* config, const QRect& rect, KisTileCommand* ktc)
{
	process( dev, dev, config, rect, ktc);
}


#endif
