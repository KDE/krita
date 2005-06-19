/*
 *  kis_tool_filter.h - part of Krita
 *
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

#ifndef __KIS_TOOL_FILTER_H__
#define __KIS_TOOL_FILTER_H__

#include "kis_tool.h"
#include "kis_tool_freehand.h"
#include "kis_tool_factory.h"

class QComboBox;
class QGridLayout;
class KisEvent;
class KisFilterConfigurationWidget;
class KisButtonPressEvent;
class KisView;
class KisID;
class KisCmbIDList;


class KisToolFilter : public KisToolFreehand {
	Q_OBJECT
	typedef KisToolFreehand super;

public:
	KisToolFilter();
	virtual ~KisToolFilter();

	virtual void setup(KActionCollection *collection);
	virtual QWidget* createOptionWidget(QWidget* parent);

public slots:
	void changeFilter( const KisID & filter);

protected:
	virtual void initPaint(KisEvent *e);

private:
	KisFilterSP m_filter;
	QWidget* m_filterConfigurationWidget;
	QGridLayout* m_optionLayout;
	KisCmbIDList * m_cbFilter;
};


class KisToolFilterFactory : public KisToolFactory {
	typedef KisToolFactory super;
public:
	KisToolFilterFactory(KActionCollection * ac) : super( ac ) {};
	virtual ~KisToolFilterFactory(){};

	virtual KisTool * createTool() { 
		KisTool * t =  new KisToolFilter();
		Q_CHECK_PTR(t);
		t -> setup(m_ac); 
		return t; 
	}
	virtual KisID id() { return KisID("filter", i18n("Filter tool")); }
};

#endif //__KIS_TOOL_FILTER_H__

