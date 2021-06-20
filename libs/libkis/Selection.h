/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_SELECTION_H
#define LIBKIS_SELECTION_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"
#include <kis_types.h>

/**
 * Selection represents a selection on Krita. A selection is
 * not necessarily associated with a particular Node or Image.
 * 
 * @code
 * from krita import *
 *
 * d = Application.activeDocument()
 * n = d.activeNode()
 * r = n.bounds() 
 * s = Selection()
 * s.select(r.width() / 3, r.height() / 3, r.width() / 3, r.height() / 3, 255)
 * s.cut(n)
 * @endcode
 */
class KRITALIBKIS_EXPORT Selection : public QObject
{
    Q_OBJECT


public:
    
    /**
     * For internal use only.
     */
    Selection(KisSelectionSP selection, QObject *parent = 0);
    
    /**
     * Create a new, empty selection object.
     */
    explicit Selection(QObject *parent = 0);
    ~Selection() override;
    
    bool operator==(const Selection &other) const;
    bool operator!=(const Selection &other) const;

public Q_SLOTS:

    /**
     * @return a duplicate of the selection
     */
    Selection *duplicate() const;

    /**
     * @return the width of the selection
     */
    int width() const;

    /**
     * @return the height of the selection
     */
    int height() const;

    /**
     * @return the left-hand position of the selection.
     */
    int x() const;

    /**
     * @return the top position of the selection.
     */
    int y() const;

    /**
     * Move the selection's top-left corner to the given coordinates.
     */
    void move(int x, int y);

    /**
     * Make the selection entirely unselected.
     */
    void clear();

    /**
     * Make the selection's width and height smaller by the given value.
     * This will not move the selection's top-left position.
     */
    void contract(int value);

    /**
     * @brief copy copies the area defined by the selection from the node to the clipboard.
     * @param node the node from where the pixels will be copied.
     */
    void copy(Node *node);

    /**
     * @brief cut erases the area defined by the selection from the node and puts a copy on the clipboard.
     * @param node the node from which the selection will be cut.
     */
    void cut(Node *node);

    /**
     * @brief paste pastes the content of the clipboard to the given node, limited by the area of the current
     * selection.
     * @param destination the node where the pixels will be written
     * @param x: the x position at which the clip will be written
     * @param y: the y position at which the clip will be written
     */
    void paste(Node *destination, int x, int y);
   
    /**
     * Erode the selection with a radius of 1 pixel.
     */
    void erode();

    /**
     * Dilate the selection with a radius of 1 pixel.
     */
    void dilate();

    /**
     * Border the selection with the given radius.
     */
    void border(int xRadius, int yRadius);

    /**
     * Feather the selection with the given radius.
     */
    void feather(int radius);

    /**
     * Grow the selection with the given radius.
     */
    void grow(int xradius, int yradius);

    /**
     * Shrink the selection with the given radius.
     */
    void shrink(int xRadius, int yRadius, bool edgeLock);

    /**
     * Smooth the selection.
     */
    void smooth();

    /** 
     * Invert the selection.
     */
    void invert();

    /**
     * Resize the selection to the given width and height. The top-left position will not be moved.
     */
    void resize(int w, int h);

    /**
     * Select the given area. The value can be between 0 and 255; 0 is 
     * totally unselected, 255 is totally selected.
     */ 
    void select(int x, int y, int w, int h, int value);

    /**
     * Select all pixels in the given node. The value can be between 0 and 255; 0 is 
     * totally unselected, 255 is totally selected.
     */
    void selectAll(Node *node, int value);

    /**
     * Replace the current selection's selection with the one of the given selection.
     */
    void replace(Selection *selection);

    /**
     * Add the given selection's selected pixels to the current selection.
     */
    void add(Selection *selection);
    
    /**
     * Subtract the given selection's selected pixels from the current selection.
     */
    void subtract(Selection *selection);

    /**
     * Intersect the given selection with this selection.
     */
    void intersect(Selection *selection);

        /**
     * Intersect with the inverse of the given selection with this selection.
     */
    void symmetricdifference(Selection *selection);

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
    friend class Document;
    friend class FilterLayer;
    friend class FillLayer;
    friend class SelectionMask;

    KisSelectionSP selection() const;

    struct Private;
    Private *const d;

};

#endif // LIBKIS_SELECTION_H
