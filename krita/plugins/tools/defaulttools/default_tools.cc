/*
 * default_tools.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
#include <QStringList>

#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_tool.h>
#include <KoToolRegistry.h>
#include "default_tools.h"

//#include "kis_tool_fill.h"
#include "kis_tool_brush.h"
#include "kis_tool_freehand.h"
#include "kis_tool_gradient.h"
#include "kis_tool_rectangle.h"
//#include "kis_tool_colorpicker.h"
#include "kis_tool_line.h"
//#include "kis_tool_text.h"
//#include "kis_tool_duplicate.h"
#include "kis_tool_move.h"
//#include "kis_tool_zoom.h"
#include "kis_tool_ellipse.h"
//#include "kis_tool_pan.h"


typedef KGenericFactory<DefaultTools> DefaultToolsFactory;
K_EXPORT_COMPONENT_FACTORY( kritadefaulttools, DefaultToolsFactory( "krita" ) )


DefaultTools::DefaultTools(QObject *parent, const QStringList &)
    : QObject(parent)
{
    KoToolRegistry * r = KoToolRegistry::instance();

    //r->add(KoToolFactorySP(new KisToolFillFactory()));
    r->add(new KisToolGradientFactory(r, QStringList()));
    r->add(new KisToolBrushFactory(r, QStringList()));
    //r->add(KoToolFactorySP(new KisToolColorPickerFactory()));
    r->add(new KisToolLineFactory(r, QStringList()));
    //r->add(KoToolFactorySP(new KisToolTextFactory()));
    //r->add(KoToolFactorySP(new KisToolDuplicateFactory()));
    r->add(new KisToolMoveFactory(r, QStringList()));
    //r->add(KoToolFactorySP(new KisToolZoomFactory()));
    r->add(new KisToolEllipseFactory(r, QStringList()));
    r->add(new KisToolRectangleFactory(r, QStringList()));
    //r->add(KoToolFactorySP(new KisToolPanFactory()));
}

DefaultTools::~DefaultTools()
{
}

#include "default_tools.moc"
