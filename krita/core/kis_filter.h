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

class KisFilterConfigurationWidget;
class KisPreviewDialog;

/**
 * Convenience function that creates an instance of a filter and adds it to the
 * filter registry of the specified view.
 */
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



/**
 * Empty interface for passing filter configuration data
 * from the configuration widget to the filter.
 */
class KisFilterConfiguration {
};



/**
 * Basic interface of a Krita filter.
 */
class KisFilter : public KisProgressSubject, public KShared {
	Q_OBJECT
public:

	KisFilter(const QString& name, KisView * view);
	virtual ~KisFilter() {}

public:

	virtual void process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration*, const QRect&) = 0;

public:
	virtual KisFilterConfiguration* configuration(KisFilterConfigurationWidget*);

	virtual bool supportsPainting() = 0;
	virtual void enableProgress();
	virtual void disableProgress();
	virtual void setAutoUpdate(bool set);

	inline const QString name() const { return m_name; };

	virtual KisFilterConfigurationWidget* createConfigurationWidget(QWidget* parent);

	virtual void cancel() { m_cancelRequested = true; }

	inline KisView* view();

public slots:

	void slotActivated();

private slots:

	void refreshPreview();

protected:

  	KisStrategyColorSpaceSP colorStrategy();
	bool m_cancelRequested;
	bool m_progressEnabled;

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




#endif
