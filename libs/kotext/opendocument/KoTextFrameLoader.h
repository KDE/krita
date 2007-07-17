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

#ifndef KOTEXTFRAMELOADER_H
#define KOTEXTFRAMELOADER_H

#include <QObject>
#include <KoStore.h>
#include <KoXmlReader.h>

//#include "KoText.h"
#include "kotext_export.h"

//class KWDocument;
//class KWTextFrameSet;
//class KWFrameSet;
//class KWFrame;
//class KWPageSettings;
//class KWPageManager;
//class KWTextFrameSet;

//class KoParagraphStyle;
//class KoCharacterStyle;
//class KoStore;
class KoTextLoader;
class KoTextLoadingContext;
//class KoStyleManager;
class KoShape;
//class KoImageCollection;
//class KoImageData;
class KoTextAnchor;

//class QDomDocument;
class QTextCursor;
//class QColor;

class KOTEXT_EXPORT KoTextFrameLoader
{
    public:
        explicit KoTextFrameLoader(KoTextLoader* loader);
        virtual ~KoTextFrameLoader();

        /**
        * Load the frame element \p frameElem into the \p cursor .
        */
        virtual void loadFrame(KoTextLoadingContext& context, const KoXmlElement& frameElem, QTextCursor& cursor);

        /**
        * Load the image frame into the \p cursor .
        */
        virtual void loadImage(KoTextLoadingContext& context, const KoXmlElement& frameElem, const KoXmlElement& imageElem, QTextCursor& cursor);

        /**
        * Load the image and return a KoShape instance for it.
        *
        * The following code provides a sample how loading an image may work. You may
        * also like to take a look at KWord how KWImageFrame is used there to lazy
        * load the image.
        * \code
        * KoShapeFactory *factory = KoShapeRegistry::instance()->value("PictureShape");
        * KoShape *shape = factory ? factory->createDefaultShape() : 0;
        * if( ! shape ) return 0;
        * KoStore* store = context.store();
        * QString href = imageElem.attribute("href");
        * if( ! store->hasFile(href) ) return shape;
        * if( store->isOpen() ) return shape;
        * if( ! store->open(href) ) return shape;
        * KoImageCollection* imagecollection = new KoImageCollection();
        * imagecollection->loadFromStore(store);
        * KoImageData* imagedata = new KoImageData(imagecollection);
        * //bool ok = imagedata->loadFromStore( store->device() );
        * shape->setUserData( imagedata );
        * store->close();
        * \endcode
        */
        virtual KoShape* loadImageShape(KoTextLoadingContext& context, const KoXmlElement& frameElem, const KoXmlElement& imageElem, QTextCursor& cursor) = 0;

    private:
        /// \internal d-pointer class.
        class Private;
        /// \internal d-pointer instance.
        Private* const d;
};

#endif
