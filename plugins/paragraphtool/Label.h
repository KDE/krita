/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
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

#ifndef LABEL_H
#define LABEL_H


#include <QColor>
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <Qt>

class QPainter;

/* simple label for displaying text */
class Label : public QObject
{
    Q_OBJECT
public:
    Label(QObject *parent = NULL);
    ~Label();

    void setColor(const QColor& color);
    void setText(const QString& text);
    void setPosition(const QPointF& position, Qt::Alignment alignment = 0x0);

    QRectF boundingRect() const;
    void paint(QPainter &painter) const;

private:
    QString m_text;
    QColor m_color;
    QPointF m_position;
    Qt::Alignment m_alignment;
};

#endif

