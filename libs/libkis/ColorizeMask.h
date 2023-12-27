/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_COLORIZEMASK_H
#define LIBKIS_COLORIZEMASK_H

#include <QObject>
#include "Node.h"
#include "ManagedColor.h"

#include <kis_types.h>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * @brief The ColorizeMask class
 * A colorize mask is a mask type node that can be used
 * to color in line art.
 *
@code
window = Krita.instance().activeWindow()
doc = Krita.instance().createDocument(10, 3, "Test", "RGBA", "U8", "", 120.0)
window.addView(doc)
root = doc.rootNode();
node = doc.createNode("layer", "paintLayer")
root.addChildNode(node, None)
nodeData = QByteArray.fromBase64(b"AAAAAAAAAAAAAAAAEQYMBhEGDP8RBgz/EQYMAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAARBgz5EQYM/xEGDAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEQYMAhEGDAkRBgwCAAAAAAAAAAAAAAAA");
node.setPixelData(nodeData,0,0,10,3)

cols = [ ManagedColor('RGBA','U8',''), ManagedColor('RGBA','U8','') ]
cols[0].setComponents([0.65490198135376, 0.345098048448563, 0.474509805440903, 1.0]);
cols[1].setComponents([0.52549022436142, 0.666666686534882, 1.0, 1.0]);
keys = [
        QByteArray.fromBase64(b"/48AAAAAAAAAAAAAAAAAAAAAAACmCwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"),
        QByteArray.fromBase64(b"AAAAAAAAAACO9ocAAAAAAAAAAAAAAAAAAAAAAMD/uQAAAAAAAAAAAAAAAAAAAAAAGoMTAAAAAAAAAAAA")
        ]

mask = doc.createColorizeMask('c1')
node.addChildNode(mask,None)
mask.setEditKeyStrokes(True)

mask.setUseEdgeDetection(True)
mask.setEdgeDetectionSize(4.0)
mask.setCleanUpAmount(70.0)
mask.setLimitToDeviceBounds(True)
mask.initializeKeyStrokeColors(cols)

for col,key in zip(cols,keys):
    mask.setKeyStrokePixelData(key,col,0,0,20,3)

mask.updateMask()
mask.setEditKeyStrokes(False);
mask.setShowOutput(True);
@endcode
 */
class KRITALIBKIS_EXPORT ColorizeMask : public Node
{
    Q_OBJECT
    Q_DISABLE_COPY(ColorizeMask)

public:
    explicit ColorizeMask(KisImageSP image, QString name, QObject *parent = 0);
    explicit ColorizeMask(KisImageSP image, KisColorizeMaskSP mask, QObject *parent = 0);
    ~ColorizeMask() override;
public Q_SLOTS:

    /**
     * @brief type Krita has several types of nodes, split in layers and masks. Group
     * layers can contain other layers, any layer can contain masks.
     *
     * @return colorizemask
     *
     * If the Node object isn't wrapping a valid Krita layer or mask object, and
     * empty string is returned.
     */
    virtual QString type() const override;

    /**
     * @brief keyStrokesColors
     * Colors used in the Colorize Mask's keystrokes.
     * @return a ManagedColor list containing the colors of keystrokes.
     */
    QList<ManagedColor*> keyStrokesColors() const;

    /**
     * @brief initializeKeyStrokeColors
     * Set the colors to use for the Colorize Mask's keystrokes.
     * @param colors a list of ManagedColor to use for the keystrokes.
     * @param transparentIndex index of the color that should be marked as transparent.
     */
    void initializeKeyStrokeColors(QList<ManagedColor*> colors, int transparentIndex = -1);

    /**
     * @brief removeKeyStroke
     * Remove a color from the Colorize Mask's keystrokes.
     * @param color a ManagedColor to be removed from the keystrokes.
     */
    void removeKeyStroke(ManagedColor* color);

    /**
     * @brief transparencyIndex
     * Index of the transparent color.
     * @return an integer containing the index of the current color marked as transparent.
     */
    int transparencyIndex() const;

    /**
     * @brief keyStrokePixelData
     * reads the given rectangle from the keystroke image data and returns it as a byte
     * array. The pixel data starts top-left, and is ordered row-first.
     * @param color a ManagedColor to get keystrokes pixeldata from.
     * @param x x position from where to start reading
     * @param y y position from where to start reading
     * @param w row length to read
     * @param h number of rows to read
     * @return a QByteArray with the pixel data. The byte array may be empty.
     */
    QByteArray keyStrokePixelData(ManagedColor* color, int x, int y, int w, int h) const;

    /**
     * @brief setKeyStrokePixelData
     * writes the given bytes, of which there must be enough, into the
     * keystroke, the keystroke's original pixels are overwritten
     *
     * @param value the byte array representing the pixels. There must be enough bytes available.
     * Krita will take the raw pointer from the QByteArray and start reading, not stopping before
     * (number of channels * size of channel * w * h) bytes are read.
     *
     * @param color a ManagedColor to set keystrokes pixeldata for.
     * @param x the x position to start writing from
     * @param y the y position to start writing from
     * @param w the width of each row
     * @param h the number of rows to write
     * @return true if writing the pixeldata worked
     */
    bool setKeyStrokePixelData(QByteArray value, ManagedColor* color, int x, int y, int w, int h);

    /**
     * @brief setUseEdgeDetection
     * Activate this for line art with large solid areas, for example shadows on an object.
     * @param value true to enable edge detection, false to disable.
     */
    void setUseEdgeDetection(bool value);

    /**
     * @brief useEdgeDetection
     * @return true if Edge detection is enabled, false if disabled.
     */
    bool useEdgeDetection() const;

    /**
     * @brief setEdgeDetectionSize
     * Set the value to the thinnest line on the image.
     * @param value a float value of the edge size to detect in pixels.
     */
    void setEdgeDetectionSize(qreal value);

    /**
     * @brief edgeDetectionSize
     * @return a float value of the edge detection size in pixels.
     */
    qreal edgeDetectionSize() const;

    /**
     * @brief setCleanUpAmount
     * This will attempt to handle messy strokes that overlap the line art where they shouldn't.
     * @param value a float value from 0.0 to 100.00 where 0.0 is no cleanup is done and 100.00 is most aggressive.
     */
    void setCleanUpAmount(qreal value);

    /**
     * @brief cleanUpAmount
     * @return a float value of 0.0 to 100.0 representing the cleanup amount where 0.0 is no cleanup is done and 100.00 is most aggressive.
     */
    qreal cleanUpAmount() const;

    /**
     * @brief setLimitToDeviceBounds
     * Limit the colorize mask to the combined layer bounds of the strokes and the line art it is filling. This can speed up the use of the mask on complicated compositions, such as comic pages.
     * @param value set true to enabled limit bounds, false to disable.
     */
    void setLimitToDeviceBounds(bool value);

    /**
     * @brief limitToDeviceBounds
     * @return true if limit bounds is enabled, false if disabled.
     */
    bool limitToDeviceBounds() const;

    /**
     * @brief updateMask
     * Process the Colorize Mask's keystrokes and generate a projection of the computed colors.
     * @param force force an update
     */
    void updateMask(bool force = false);

    void resetCache();

    /**
     * @brief showOutput
     * Show output mode allows the user to see the result of the Colorize Mask's algorithm.
     * @return true if edit show coloring mode is enabled, false if disabled.
     */
    bool showOutput() const;

    /**
     * @brief setShowOutput
     * Toggle Colorize Mask's show output mode.
     * @param enabled set true to enable show coloring mode and false to disable it.
     */
    void setShowOutput(bool enabled);

    /**
     * @brief editKeyStrokes
     * Edit keystrokes mode allows the user to modify keystrokes on the active Colorize Mask.
     * @return true if edit keystrokes mode is enabled, false if disabled.
     */
    bool editKeyStrokes() const;

    /**
     * @brief setEditKeyStrokes
     * Toggle Colorize Mask's edit keystrokes mode.
     * @param enabled set true to enable edit keystrokes mode and false to disable it.
     */
    void setEditKeyStrokes(bool enabled);

};

#endif // LIBKIS_COLORIZEMASK_H


