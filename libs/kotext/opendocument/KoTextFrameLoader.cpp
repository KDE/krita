/* This file is part of the KDE project
 * Copyright (C) 2004-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
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

#include "KoTextFrameLoader.h"
#include "KoTextLoader.h"
#include "KoTextLoadingContext.h"
//#include "KWDocument.h"
//#include "frames/KWTextFrameSet.h"
//#include "frames/KWTextFrame.h"

// koffice
#include <KoOasisStyles.h>
#include <KoOasisSettings.h>
#include <KoXmlNS.h>
#include <KoDom.h>
#include <KoUnit.h>
#include <KoPageLayout.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactory.h>
#include <KoShape.h>
#include <KoShapeLoadingContext.h>
#include <KoImageData.h>
#include <KoTextAnchor.h>
#include <KoTextDocumentLayout.h>
#include <KoVariableManager.h>
#include <KoInlineTextObjectManager.h>
#include <KoInlineObjectRegistry.h>
#include <KoProperties.h>
#include <KoVariable.h>

#include "../styles/KoStyleManager.h"
#include "../styles/KoParagraphStyle.h"
#include "../styles/KoCharacterStyle.h"
#include "../styles/KoListStyle.h"
#include "../styles/KoListLevelProperties.h"

// KDE + Qt includes
#include <QDomDocument>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextList>
#include <klocale.h>

// if defined then debugging is enabled
#define KOOPENDOCUMENTLOADER_DEBUG

/// \internal d-pointer class.
class KoTextFrameLoader::Private
{
    public:
        KoTextLoader* loader;
        //QList<KoTextAnchor*> anchors;
};

KoTextFrameLoader::KoTextFrameLoader(KoTextLoader* loader)
    : d(new Private())
{
    d->loader = loader;
}

KoTextFrameLoader::~KoTextFrameLoader()
{
    delete d;
}

void KoTextFrameLoader::loadFrame(KoTextLoadingContext& context, const KoXmlElement& frameElem, QTextCursor& cursor)
{
    for(KoXmlNode node = frameElem.firstChild(); !node.isNull(); node = node.nextSibling()) {
        KoXmlElement ts = node.toElement();
        if( ts.isNull() ) continue;
        const QString localName( ts.localName() );
        //const bool isTextNS = ( ts.namespaceURI() == KoXmlNS::text );
        const bool isDrawNS = ( ts.namespaceURI() == KoXmlNS::draw );
        if (isDrawNS && localName == "image") {
            loadImage(context, frameElem, ts, cursor);
        }
        else {
            kWarning(32500) << "KoTextFrameLoader::loadFrame Unhandled frame: " << localName;
        }
    }
}

void KoTextFrameLoader::loadImage(KoTextLoadingContext& context, const KoXmlElement& _frameElem, const KoXmlElement& _imageElem, QTextCursor& cursor)
{
    const KoXmlElement frameElem = _frameElem;
    const KoXmlElement imageElem = _imageElem;
    KoShape* shape = loadImageShape(context, frameElem, imageElem, cursor);
    if( ! shape ) {
        kWarning(32500) << "KoTextFrameLoader::loadImage Failed to create picture shape";
        return;
    }
}

