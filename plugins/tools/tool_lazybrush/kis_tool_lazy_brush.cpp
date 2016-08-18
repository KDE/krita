/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_tool_lazy_brush.h"

#include <klocalizedstring.h>
#include <QAction>
#include <QLabel>
#include <kactioncollection.h>

#include <KoCanvasBase.h>
#include <KoCanvasController.h>

#include "kis_cursor.h"
#include "kis_config.h"
#include "kundo2magicstring.h"

KisToolLazyBrush::KisToolLazyBrush(KoCanvasBase * canvas)
    : KisToolFreehand(canvas,
                      KisCursor::load("tool_freehand_cursor.png", 5, 5),
                      kundo2_i18n("Colorize Mask Key Stroke"))
{
    setObjectName("tool_lazybrush");
}

KisToolLazyBrush::~KisToolLazyBrush()
{
}

void KisToolLazyBrush::activate(ToolActivation activation, const QSet<KoShape*> &shapes)
{
    KisToolFreehand::activate(activation, shapes);
}

void KisToolLazyBrush::deactivate()
{
    KisToolFreehand::deactivate();
}

void KisToolLazyBrush::resetCursorStyle()
{
    KisToolFreehand::resetCursorStyle();
}

QWidget * KisToolLazyBrush::createOptionWidget()
{
    QWidget *optionsWidget = KisToolFreehand::createOptionWidget();
    optionsWidget->setObjectName(toolId() + "option widget");

    // // See https://bugs.kde.org/show_bug.cgi?id=316896
    // QWidget *specialSpacer = new QWidget(optionsWidget);
    // specialSpacer->setObjectName("SpecialSpacer");
    // specialSpacer->setFixedSize(0, 0);
    // optionsWidget->layout()->addWidget(specialSpacer);


    return optionsWidget;
}


