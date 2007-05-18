/* This file is part of the KDE project
 * Copyright (C) 2005 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
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

#ifndef KOKoOpenDocumentLoader_H
#define KOKoOpenDocumentLoader_H

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
class KoOasisStyles;
class KoOasisLoadingContext;
class KoStyleManager;

//class QDomDocument;
class QTextCursor;
//class QColor;

/**
 * The KoOpenDocumentLoader loads content from OpenDocument XML to the
 * scribe text-engine using QTextCursor objects.
 *
 * This class that has a lot of the OpenDocument (ODF) loading code
 * for e.g. KWord.
 */
class KOTEXT_EXPORT KoOpenDocumentLoader : public QObject
{
        Q_OBJECT
    public:

        /**
        * Constructor.
        * \param stylemanager the \a KoStyleManager instance the loader
        * should use.
        */
        explicit KoOpenDocumentLoader(KoStyleManager* stylemanager);

        /**
        * Destructor.
        */
        virtual ~KoOpenDocumentLoader();

        /**
        * \return the \a KoStyleManager instance this loader uses.
        */
        KoStyleManager* styleManager() const;

        /**
        * Load the list of styles.
        */
        virtual void loadStyles(KoOasisLoadingContext& context, QList<KoXmlElement*> styleElements);

        /**
        * Load all styles. This includes the auto-styles and the custom-styles.
        */
        virtual void loadAllStyles(KoOasisLoadingContext& context);

        /**
        * Load the settings.
        */
        virtual void loadSettings(KoOasisLoadingContext& context, const QDomDocument& settings);

        /**
        * Load the page layout.
        */
        virtual bool loadPageLayout(KoOasisLoadingContext& context, const QString& masterPageName);

        /**
        * Load the style of the masterpage.
        */
        virtual bool loadMasterPageStyle(KoOasisLoadingContext& context, const QString& masterPageName);

        /**
        * Load the body from the \p bodyElem into the \p cursor .
        */
        virtual void loadBody(KoOasisLoadingContext& context, const KoXmlElement& bodyElem, QTextCursor& cursor);

        /**
        * Load the paragraph from the \p paragraphElem into the \p cursor .
        */
        virtual void loadParagraph(KoOasisLoadingContext& context, const KoXmlElement& paragraphElem, QTextCursor& cursor);

        /**
        * Load the heading from the \p headingElem into the \p cursor .
        */
        virtual void loadHeading(KoOasisLoadingContext& context, const KoXmlElement& headingElem, QTextCursor& cursor);

        /**
        * Load the list from the \p listElem into the \p cursor .
        */
        virtual void loadList(KoOasisLoadingContext& context, const KoXmlElement& listElem, QTextCursor& cursor);

        /**
        * Load the section from the \p sectionElem into the \p cursor .
        */
        virtual void loadSection(KoOasisLoadingContext& context, const KoXmlElement& sectionElem, QTextCursor& cursor);

        /**
        * Load the span from the \p spanElem into the \p cursor .
        */
        virtual void loadSpan(KoOasisLoadingContext& context, const KoXmlElement& spanElem, QTextCursor& cursor, bool* stripLeadingSpace);

        //virtual void loadFrame(KoOasisLoadingContext& context, const KoXmlElement& frameElem, QTextCursor& cursor);

    signals:

        /**
        * This signal is emitted during loading with a percentage within 1-100 range
        * \param percent the progress as a percentage
        */
        void sigProgress(int percent);

    private:
        /// \internal d-pointer class.
        class Private;
        /// \internal d-pointer instance.
        Private* const d;
};

#endif
