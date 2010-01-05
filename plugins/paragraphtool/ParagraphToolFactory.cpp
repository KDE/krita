/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ParagraphToolFactory.h"
#include "ParagraphTool.h"

#include <klocale.h>

ParagraphToolFactory::ParagraphToolFactory(QObject *parent)
        : KoToolFactory(parent, "ParagraphToolFactory_ID")
{
    setToolTip(i18n("Paragraph tool"));
    setToolType(dynamicToolType());
    setIcon("draw-text");
    setPriority(2);
    setActivationShapeId("TextShapeID");
}

ParagraphToolFactory::~ParagraphToolFactory()
{
}

KoTool * ParagraphToolFactory::createTool(KoCanvasBase *canvas)
{
    return new ParagraphTool(canvas);
}

#include "ParagraphToolFactory.moc"
