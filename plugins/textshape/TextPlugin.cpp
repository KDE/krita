/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#include "TextPlugin.h"
#include "TextToolFactory.h"
#include "ReferencesToolFactory.h"
#include "ReviewToolFactory.h"
#ifdef CREATE_TEXTDOCUMENT_INSPECTOR
#include "TextDocumentInspectionPlugin.h"
#endif
#include "TextShapeFactory.h"
#include "AnnotationTextShapeFactory.h"

#include <KoShapeRegistry.h>
#include <KoDockRegistry.h>
#include <KoToolRegistry.h>

#include <kpluginfactory.h>

#ifdef CREATE_TEXTDOCUMENT_INSPECTOR
K_PLUGIN_FACTORY_WITH_JSON(TextPluginFactory, "calligra_shape_text.json", registerPlugin<TextPlugin>(); registerPlugin<TextDocumentInspectionPlugin>(QLatin1String("TextDocumentInspection"));)
#else
K_PLUGIN_FACTORY_WITH_JSON(TextPluginFactory, "calligra_shape_text.json", registerPlugin<TextPlugin>();)
#endif

TextPlugin::TextPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoToolRegistry::instance()->add(new TextToolFactory());
    KoToolRegistry::instance()->add(new ReviewToolFactory());
    KoToolRegistry::instance()->add(new ReferencesToolFactory());
    KoShapeRegistry::instance()->add(new TextShapeFactory());
    KoShapeRegistry::instance()->add(new AnnotationTextShapeFactory());
}

#include <TextPlugin.moc>
