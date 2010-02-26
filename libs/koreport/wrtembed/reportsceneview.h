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

#ifndef __REPORTSCENEVIEW_H__
#define __REPORTSCENEVIEW_H__

#include <QGraphicsView>
class KoReportDesigner;

class ReportSceneView : public QGraphicsView
{
    Q_OBJECT
public:
    ReportSceneView(KoReportDesigner *, QGraphicsScene * scene, QWidget * parent = 0, const char * name = 0);
    virtual ~ReportSceneView();

    KoReportDesigner * designer();
    virtual QSize sizeHint() const;
public slots:
    void resizeContents(QSize);

protected:
    void mouseReleaseEvent(QMouseEvent * e);

private:
    KoReportDesigner* m_reportDesigner;
};

#endif
