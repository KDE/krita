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
#include <kis_types.h>

/**
 * Selection
 */
class KRITALIBKIS_EXPORT Selection : public QObject
{
    Q_OBJECT

public:
    Selection(KisSelectionSP selection, QObject *parent = 0);
    explicit Selection(QObject *parent = 0);
    virtual ~Selection();

public Q_SLOTS:

    int width() const;

    int height() const;

    int x() const;

    int y() const;

    void move(int x, int y);

    void clear();

    void contract(int value);

    void cut(Node* node);

    void paste(Node *source, Node*destination);

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

    /**
     * @brief pixelData reads the given rectangle from the Selection's mask and returns it as a
     * byte array. The pixel data starts top-left, and is ordered row-first.
     *
     * The byte array will contain one byte for every pixel, representing the selectedness. 0
     * is totally unselected, 255 is fully selected.
     *
     * You can read outside the Selection's boundaries; those pixels will be unselected.
     *
     * The byte array is a copy of the original selection data.
     * @param x x position from where to start reading
     * @param y y position from where to start reading
     * @param w row length to read
     * @param h number of rows to read
     * @return a QByteArray with the pixel data. The byte array may be empty.
     */
    QByteArray pixelData(int x, int y, int w, int h) const;

    /**
     * @brief setPixelData writes the given bytes, of which there must be enough, into the
     * Selection.
     *
     * @param value the byte array representing the pixels. There must be enough bytes available.
     * Krita will take the raw pointer from the QByteArray and start reading, not stopping before
     * (w * h) bytes are read.
     *
     * @param x the x position to start writing from
     * @param y the y position to start writing from
     * @param w the width of each row
     * @param h the number of rows to write
     */
    void setPixelData(QByteArray value, int x, int y, int w, int h);

private:
    struct Private;
    Private *const d;

};

#endif // LIBKIS_SELECTION_H
