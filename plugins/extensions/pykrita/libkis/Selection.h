/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef LIBKIS_SELECTION_H
#define LIBKIS_SELECTION_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * Selection
 */
class KRITALIBKIS_EXPORT Selection : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Selection)
    
    Q_PROPERTY(int Width READ width WRITE setWidth)
    Q_PROPERTY(int Height READ height WRITE setHeight)
    Q_PROPERTY(int X READ x WRITE setX)
    Q_PROPERTY(int Y READ y WRITE setY)
    Q_PROPERTY(QString Type READ type WRITE setType)

public:
    explicit Selection(QObject *parent = 0);
    virtual ~Selection();

    int width() const;
    void setWidth(int value);

    int height() const;
    void setHeight(int value);

    int x() const;
    void setX(int value);

    int y() const;
    void setY(int value);

    QString type() const;
    void setType(QString value);



public Q_SLOTS:
    
    void clear();

    void contract(int value);

    Selection* copy(int x, int y, int w, int h);

    void cut(Node* node);

    void deselect();

    void expand(int value);

    void feather(int value);

    void fill(Node* node);

    void grow(int value);

    void invert();

    void resize(int w, int h);

    void rotate(int degrees);

    void select(int x, int y, int w, int h, int value);

    void selectAll(Node *node);


    
Q_SIGNALS:



private:
    struct Private;
    const Private *const d;

};

#endif // LIBKIS_SELECTION_H
