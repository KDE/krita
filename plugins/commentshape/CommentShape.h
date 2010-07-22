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

#ifndef COMMENTSHAPE_H
#define COMMENTSHAPE_H
#include <KoShape.h>
#include <QDate>

class CommentShape : public KoShape
{
public:
    CommentShape();
    virtual ~CommentShape();

    virtual bool loadOdf(const KoXmlElement& element, KoShapeLoadingContext& context);
    virtual void paint(QPainter& painter, const KoViewConverter& converter);
    virtual void saveOdf(KoShapeSavingContext& context) const;

    virtual void setSize(const QSizeF& size);
private:
    QString m_comment;
    QString m_creator;
    QDate m_date;
};

#endif // COMMENTSHAPE_H
