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
#include <stdlib.h>
#include <vector>

#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_global.h>
#include <kis_types.h>
#include <kis_tool.h>
#include <kis_tool_registry.h>
#include "default_tools.h"

#include "kis_tool_fill.h"
#include "kis_tool_brush.h"
#include "kis_tool_freehand.h"
#include "kis_tool_gradient.h"
#include "kis_tool_rectangle.h"
#include "kis_tool_colorpicker.h"
#include "kis_tool_line.h"
#include "kis_tool_text.h"
#include "kis_tool_duplicate.h"
#include "kis_tool_move.h"
#include "kis_tool_zoom.h"
#include "kis_tool_ellipse.h"
#include "kis_tool_pan.h"


typedef KGenericFactory<DefaultTools> DefaultToolsFactory;
K_EXPORT_COMPONENT_FACTORY( kritadefaulttools, DefaultToolsFactory( "krita" ) )


DefaultTools::DefaultTools(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent)
{
    setObjectName(name);
    setInstance(DefaultToolsFactory::instance());

    if ( parent->inherits("KisToolRegistry") )
    {
        KisToolRegistry * r = dynamic_cast<KisToolRegistry*>(parent);

        r->add(KisToolFactorySP(new KisToolFillFactory()));
        r->add(KisToolFactorySP(new KisToolGradientFactory()));
        r->add(KisToolFactorySP(new KisToolBrushFactory()));
        r->add(KisToolFactorySP(new KisToolColorPickerFactory()));
        r->add(KisToolFactorySP(new KisToolLineFactory()));
        r->add(KisToolFactorySP(new KisToolTextFactory()));
        r->add(KisToolFactorySP(new KisToolDuplicateFactory()));
        r->add(KisToolFactorySP(new KisToolMoveFactory()));
        r->add(KisToolFactorySP(new KisToolZoomFactory()));
        r->add(KisToolFactorySP(new KisToolEllipseFactory()));
        r->add(KisToolFactorySP(new KisToolRectangleFactory()));
        r->add(KisToolFactorySP(new KisToolPanFactory()));
    }
}

DefaultTools::~DefaultTools()
{
}

#include "default_tools.moc"
