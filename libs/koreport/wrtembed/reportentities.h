/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *     This file contains all the Report Entity classes. Each one is a
 * derivative of ReportEntity, which in turn is derived from QCanvasItem.
 */

#ifndef __REPORTENTITIES_H__
#define __REPORTENTITIES_H__

// qt
#include <QGraphicsItem>
// common
#include <krobjectdata.h>


// forward declarations
class ReportWindow;

class QDomNode;
class QDomDocument;
class QDomElement;
class KoReportDesigner;
class KRPos;
class KRSize;

namespace KoProperty
{
class Editor;
}

//
// ReportEntity
//
class ReportEntity
{
public:
    static void buildXML(QGraphicsItem * item, QDomDocument & doc, QDomElement & parent);
    virtual void buildXML(QDomDocument & doc, QDomElement & parent) = 0;

    static void buildXMLRect(QDomDocument & doc, QDomElement & entity, KRPos *pos, KRSize *siz);
    static void buildXMLTextStyle(QDomDocument & doc, QDomElement & entity, KRTextStyleData ts);
    static void buildXMLLineStyle(QDomDocument & doc, QDomElement & entity, KRLineStyleData ls);

    static QFont getDefaultEntityFont();
    static void  setDefaultEntityFont(const QFont &);

    virtual ReportEntity* clone() = 0;
    KoReportDesigner* designer() const {
        return m_reportDesigner;
    }
    void setDesigner(KoReportDesigner* rd) {
        m_reportDesigner = rd;
    }
    virtual ~ReportEntity() {};

    static void addPropertyAsAttribute(QDomElement* e, KoProperty::Property* p);

protected:
    ReportEntity(KoReportDesigner*);
    KoReportDesigner* m_reportDesigner;
    QString dataSourceAndObjectTypeName(const QString& dataSource, const QString& objectTypeName);
private:
    static bool m_readDefaultFont;
    static QFont m_defaultFont;
};

#endif

