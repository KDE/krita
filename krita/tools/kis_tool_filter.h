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

class QComboBox;
class QGridLayout;
class KisEvent;
class KisFilterConfigurationWidget;
class KisButtonPressEvent;
class KisView;

class KisToolFilter : public KisToolFreeHand {
	Q_OBJECT
	typedef KisToolFreeHand super;

public:
	KisToolFilter(KisView* view);
	virtual ~KisToolFilter();
  
	virtual void setup(KActionCollection *collection);
	virtual QWidget* createOptionWidget(QWidget* parent);
	virtual QWidget* optionWidget();

public slots:
	void changeFilter( const QString & string );

protected:
	virtual void initPaint(KisEvent *e);

private:
	KisView* m_view;
	KisFilterSP m_filter;
	KisFilterConfigurationWidget* m_filterConfigurationWidget;
	QGridLayout* m_optionLayout;
	QWidget* m_optWidget;
	QComboBox* m_cbFilter;
};

#endif //__KIS_TOOL_FILTER_H__

