/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2008 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOFRAMESHAPE_H
#define KOFRAMESHAPE_H

#include "kritaflake_export.h"

class KoShapeLoadingContext;
#include <QDomDocument>
class QString;

/**
 * @brief Base class for shapes that are saved as a part of a draw:frame.
 *
 * Shapes like the TextShape or the PictureShape are implementing this
 * class to deal with frames and their attributes.
 *
 * What follows is a sample taken out of an ODT-file that shows how this works
 * together;
 * @code
 * <draw:frame draw:style-name="fr1" text:anchor-type="paragraph" svg:x="0.6429in" svg:y="0.1409in" svg:width="4.7638in" svg:height="3.3335in">
 *   <draw:image xlink:href="Pictures/imagename.jpg" />
 * </draw:frame>
 * @endcode
 */
class KRITAFLAKE_EXPORT KoFrameShape
{
public:

    /**
    * Constructor.
    *
    * \param ns The namespace. E.g. KoXmlNS::draw
    * \param element The tag-name. E.g. "image"
    */
    KoFrameShape(const QString &ns, const QString &tag);

    /**
     * Copy contrustor
     */
    KoFrameShape(const KoFrameShape &rhs);

    /**
    * Destructor.
    */
    virtual ~KoFrameShape();

    /**
    * Loads the content of the draw:frame element and it's children. This
    * method calls the abstract loadOdfFrameElement() method.
    *
    * @return false if loading failed
    */
    virtual bool loadOdfFrame(const QDomElement &element, KoShapeLoadingContext &context);

protected:

    /**
    * Abstract method to handle loading of the defined inner element like
    * e.g. the draw:image element.
    * @return false if loading failed
    */
    virtual bool loadOdfFrameElement(const QDomElement &element, KoShapeLoadingContext &context) = 0;

private:
    class Private;
    Private * const d;
};

#endif /* KOFRAMESHAPE_H */
