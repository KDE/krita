/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#ifndef KOTEXTONSHAPECONTAINER_H
#define KOTEXTONSHAPECONTAINER_H

#include "KoShapeContainer.h"

#include "flake_export.h"

class KoTextOnShapeContainerPrivate;
class KoResourceManager;

/**
 * Container that is used to wrap a shape with a text on top.
 * Adding this container as a parent to any shape will allow you to add text
 * on top of that shape in the form of the decorator (design) pattern.
 */
class FLAKE_EXPORT KoTextOnShapeContainer : public KoShapeContainer
{
public:
    explicit KoTextOnShapeContainer(KoShape *childShape, KoResourceManager *documentResources = 0);

    virtual void paintComponent(QPainter &painter, const KoViewConverter &converter);
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);
    virtual void saveOdf(KoShapeSavingContext &context) const;

    /*
      Add API to set options like;
        Resize behavior
         * content follows text frame-size
         * text frame-size follows content size
        Alignment
        etc.
     */

    void setPlainText(const QString &text);

private:
    Q_DECLARE_PRIVATE(KoTextOnShapeContainer)
};

#endif
