/* This file is part of the KDE project
 * Copyright 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#include <QStringList>
#include <QFontDatabase>

#include <kpluginfactory.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <KoIcon.h>
#include <KoProperties.h>
#include <KoToolRegistry.h>
#include <KoShapeRegistry.h>
#include <KoShapeLoadingContext.h>

#include "MusicShape.h"
#include "MusicToolFactory.h"
#include "SimpleEntryToolFactory.h"

#include "MusicShapeFactory.h"

K_PLUGIN_FACTORY(MusicShapePluginFactory, registerPlugin<MusicShapePlugin>();)
K_EXPORT_PLUGIN(MusicShapePluginFactory( "MusicShape" ))

MusicShapePlugin::MusicShapePlugin( QObject *,  const QVariantList& )
{
    KoShapeRegistry::instance()->add( new MusicShapeFactory() );
    KoToolRegistry::instance()->add( new MusicToolFactory() );
    KoToolRegistry::instance()->add( new SimpleEntryToolFactory() );
}


MusicShapeFactory::MusicShapeFactory()
    : KoShapeFactoryBase(MusicShapeId, i18n( "Music Shape" ) )
{
    setToolTip( i18n( "A shape which provides a music editor" ) );
    setIconName(koIconNameCStrNeededWithSubs("icon for the Music Shape","musicshape", "music-note-16th"));
    setXmlElementNames( "http://www.calligra.org/music", QStringList("shape") );
    setLoadingPriority( 1 );
}

KoShape *MusicShapeFactory::createDefaultShape(KoDocumentResourceManager *) const
{
    static bool loadedFont = false;
    if (!loadedFont) {
        QString fontFile = KStandardDirs::locate("data", "musicshape/fonts/Emmentaler-14.ttf");
        if (QFontDatabase::addApplicationFont(fontFile) == -1) {
            kWarning() << "Could not load emmentaler font";
        }
        loadedFont = true;
    }
    MusicShape* shape = new MusicShape();
    shape->setSize(QSizeF(400, 300));
    shape->setShapeId(MusicShapeId);
    return shape;
}

bool MusicShapeFactory::supports(const KoXmlElement & e, KoShapeLoadingContext &context) const
{
    Q_UNUSED(context);
    return ( e.localName() == "shape" ) && ( e.namespaceURI() == "http://www.calligra.org/music" );
}
