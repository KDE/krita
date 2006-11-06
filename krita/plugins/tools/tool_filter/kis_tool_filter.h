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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_TOOL_FILTER_H__
#define __KIS_TOOL_FILTER_H__

#include "kis_tool_freehand.h"
#include "KoToolFactory.h"

class QComboBox;
class QGridLayout;
class KoPointerEvent;
class KisFilterConfigurationWidget;
class KoPointerEvent;
class KisView;
class KoID;
class KisCmbIDList;


class KisToolFilter : public KisToolFreehand {
    Q_OBJECT
    typedef KisToolFreehand super;

public:
    KisToolFilter();
    virtual ~KisToolFilter();

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_FREEHAND; }
    virtual QWidget* createOptionWidget(QWidget* parent);

public slots:
    void changeFilter( const KoID & filter);

protected:
    virtual void initPaint(KoPointerEvent *e);

private:
    KisFilterSP m_filter;
    QWidget* m_filterConfigurationWidget;
    QGridLayout* m_optionLayout;
    KisCmbIDList * m_cbFilter;
};


class KisToolFilterFactory : public KoToolFactory {

public:
    KisToolFilterFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolFilter", i18n( "Filter Brush" ) )
        {
            setToolTip( i18n( "Paint with a filter and the the current brush" ) );
            setToolType( TOOL_TYPE_FREEHAND );
            setIcon( "tool_filter" );
            setPriority( 0 );
        }

    virtual ~KisToolFilterFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolFilter(canvas);
    }

};

#endif //__KIS_TOOL_FILTER_H__

