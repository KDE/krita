/*
 * Copyright (C) 2008-2010 Thomas Zander <zander@kde.org>
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
#ifndef FOLDERBORDER_H
#define FOLDERBORDER_H

#include <KoShapeBorderModel.h>

/**
 * The shapeselector allows multiple 'folders' and this is the border to show the edges.
 * In the startup phase there is only one folder, which has no border. If the user adds
 * more folders the FolderShape gets a FolderBorder assigned to it which draws the outline
 * and the title of the folder shape.
 */
class FolderBorder : public KoShapeBorderModel
{
public:
    FolderBorder();
    /// reimplemented from KoShapeBorderModel
    virtual void fillStyle(KoGenStyle &, KoShapeSavingContext &) const {}
    /// reimplemented from KoShapeBorderModel
    virtual void borderInsets(const KoShape *shape, KoInsets &insets) const;
    /// reimplemented from KoShapeBorderModel
    virtual bool hasTransparency() const;
    /// reimplemented from KoShapeBorderModel
    virtual void paint(KoShape *shape, QPainter &painter, const KoViewConverter &converter);
    /// reimplemented from KoShapeBorderModel
    virtual void paint(KoShape *shape, QPainter &painter, const KoViewConverter &converter, const QColor &color);
};

#endif
