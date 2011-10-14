/* This file is part of the KDE project
 * Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
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

#ifndef INITIALSCOMMENTSHAPE_H
#define INITIALSCOMMENTSHAPE_H

#include <KoShape.h>

class InitialsCommentShape : public KoShape
{
public:
    InitialsCommentShape();
    virtual ~InitialsCommentShape();

    virtual void saveOdf(KoShapeSavingContext& context) const;
    virtual bool loadOdf(const KoXmlElement& element, KoShapeLoadingContext& context);
    virtual void paint(QPainter& painter, const KoViewConverter& converter, KoShapePaintingContext &paintcontext);

    void setInitials(QString initials);
    QString initials();

    bool isActive() const;
    void setActive(bool activate);
    void toogleActive();
private:
    bool m_active;
    QString m_initials;
};

#endif // INITIALSCOMMENTSHAPE_H
