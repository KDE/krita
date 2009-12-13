/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOSHAPEPASTE_H
#define KOSHAPEPASTE_H

#include <KoOdfPaste.h>
#include "flake_export.h"

#include <QList>

class KoCanvasBase;
class KoShapeLayer;
class KoShape;

/**
 * Class for pasting shapes to the document
 */
class FLAKE_EXPORT KoShapePaste : public KoOdfPaste
{
public:
    /**
     * Contructor
     *
     * @param canvas The canvas on which the paste is done
     * @param zIndex The highest currently existing zIndex.
     * @param parentLayer The layer on which the shapes will be pasted
     */
    KoShapePaste(KoCanvasBase *canvas, KoShapeLayer *parentLayer);
    virtual ~KoShapePaste();

    QList<KoShape*> pastedShapes() const;

protected:
    /// reimplemented
    virtual bool process(const KoXmlElement & body, KoOdfReadStore &odfStore);

    class Private;
    Private * const d;
};

#endif /* KOSHAPEPASTE_H */
