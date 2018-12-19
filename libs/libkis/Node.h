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
#ifndef LIBKIS_NODE_H
#define LIBKIS_NODE_H

#include <QObject>

#include <kis_types.h>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * Node represents a layer or mask in a Krita image's Node hierarchy. Group layers can contain
 * other layers and masks; layers can contain masks.
 *
 */
class KRITALIBKIS_EXPORT Node : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Node)

public:
    explicit Node(KisImageSP image, KisNodeSP node, QObject *parent = 0);
    ~Node() override;
    bool operator==(const Node &other) const;
    bool operator!=(const Node &other) const;

public Q_SLOTS:

    /**
     * @brief clone clone the current node. The node is not associated with any image.
     */
    Node *clone() const;

    /**
     * @brief alphaLocked checks whether the node is a paint layer and returns whether it is alpha locked
     * @return whether the paint layer is alpha locked, or false if the node is not a paint layer
     */
    bool alphaLocked() const;

    /**
     * @brief setAlphaLocked set the layer to value if the node is paint layer.
     */
    void setAlphaLocked(bool value);

    /**
     * @return the blending mode of the layer. The values of the blending modes are defined in @see KoCompositeOpRegistry.h
     */
    QString blendingMode() const;

    /**
     * @brief setBlendingMode set the blending mode of the node to the given value
     * @param value one of the string values from @see KoCompositeOpRegistry.h
     */
    void setBlendingMode(QString value);

    /**
     * @brief channels creates a list of Channel objects that can be used individually to
     * show or hide certain channels, and to retrieve the contents of each channel in a
     * node separately.
     *
     * Only layers have channels, masks do not, and calling channels on a Node that is a mask
     * will return an empty list.
     *
     * @return the list of channels ordered in by position of the channels in pixel position
     */
    QList<Channel*> channels() const;

    /**
     * Return a list of child nodes of the current node. The nodes are ordered from the bottommost up.
     * The function is not recursive.
     */
    QList<Node*> childNodes() const;

    /**
     * @brief addChildNode adds the given node in the list of children.
     * @param child the node to be added
     * @param above the node above which this node will be placed
     * @return false if adding the node failed
     */
    bool addChildNode(Node *child, Node *above);

    /**
     * @brief removeChildNode removes the given node from the list of children.
     * @param child the node to be removed
     */
    bool removeChildNode(Node *child);

    /**
     * @brief setChildNodes this replaces the existing set of child nodes with the new set.
     * @param nodes The list of nodes that will become children, bottom-up -- the first node,
     * is the bottom-most node in the stack.
     */
    void setChildNodes(QList<Node*> nodes);

    /**
     * colorDepth A string describing the color depth of the image:
     * <ul>
     * <li>U8: unsigned 8 bits integer, the most common type</li>
     * <li>U16: unsigned 16 bits integer</li>
     * <li>F16: half, 16 bits floating point. Only available if Krita was built with OpenEXR</li>
     * <li>F32: 32 bits floating point</li>
     * </ul>
     * @return the color depth.
     */
    QString colorDepth() const;

    /**
     * @brief colorModel retrieve the current color model of this document:
     * <ul>
     * <li>A: Alpha mask</li>
     * <li>RGBA: RGB with alpha channel (The actual order of channels is most often BGR!)</li>
     * <li>XYZA: XYZ with alpha channel</li>
     * <li>LABA: LAB with alpha channel</li>
     * <li>CMYKA: CMYK with alpha channel</li>
     * <li>GRAYA: Gray with alpha channel</li>
     * <li>YCbCrA: YCbCr with alpha channel</li>
     * </ul>
     * @return the internal color model string.
     */
    QString colorModel() const;

    /**
     * @return the name of the current color profile
     */
    QString colorProfile() const;

    /**
     * @brief setColorProfile set the color profile of the image to the given profile. The profile has to
     * be registered with krita and be compatible with the current color model and depth; the image data
     * is <i>not</i> converted.
     * @param colorProfile
     * @return if assigning the color profile worked
     */
    bool setColorProfile(const QString &colorProfile);

    /**
     * @brief setColorSpace convert the node to the given colorspace
     * @param colorModel A string describing the color model of the node:
     * <ul>
     * <li>A: Alpha mask</li>
     * <li>RGBA: RGB with alpha channel (The actual order of channels is most often BGR!)</li>
     * <li>XYZA: XYZ with alpha channel</li>
     * <li>LABA: LAB with alpha channel</li>
     * <li>CMYKA: CMYK with alpha channel</li>
     * <li>GRAYA: Gray with alpha channel</li>
     * <li>YCbCrA: YCbCr with alpha channel</li>
     * </ul>
     * @param colorDepth A string describing the color depth of the image:
     * <ul>
     * <li>U8: unsigned 8 bits integer, the most common type</li>
     * <li>U16: unsigned 16 bits integer</li>
     * <li>F16: half, 16 bits floating point. Only available if Krita was built with OpenEXR</li>
     * <li>F32: 32 bits floating point</li>
     * </ul>
     * @param colorProfile a valid color profile for this color model and color depth combination.
     */
    bool setColorSpace(const QString &colorModel, const QString &colorDepth, const QString &colorProfile);

    /**
     * @brief Krita layers can be animated, i.e., have frames.
     * @return return true if the layer has frames. Currently, the scripting framework
     * does not give access to the animation features.
     */
    bool animated() const;

    /**
     * @brief enableAnimation make the current layer animated, so it can have frames.
     */
    void enableAnimation() const;

    /**
     * @brief Should the node be visible in the timeline. It defaults to false
     * with new layer
     */
    void setShowInTimeline(bool showInTimeline) const;

    /**
     * @return is layer is shown in the timeline
     */
    bool showInTimeline() const;


    /**
     * Sets the state of the node to the value of @param collapsed
     */
    void setCollapsed(bool collapsed);

    /**
     * returns the collapsed state of this node
     */
    bool collapsed() const;

    /**
     * Sets a color label index associated to the layer.  The actual
     * color of the label and the number of available colors is
     * defined by Krita GUI configuration.
     */
    int colorLabel() const;

    /**
     * @brief setColorLabel sets a color label index associated to the layer.  The actual
     * color of the label and the number of available colors is
     * defined by Krita GUI configuration.
     * @param index an integer corresponding to the set of available color labels.
     */
    void setColorLabel(int index);

    /**
     * @brief inheritAlpha checks whether this node has the inherits alpha flag set
     * @return true if the Inherit Alpha is set
     */
    bool inheritAlpha() const;

    /**
     * set the Inherit Alpha flag to the given value
     */
    void setInheritAlpha(bool value);

    /**
     * @brief locked checks whether the Node is locked. A locked node cannot be changed.
     * @return true if the Node is locked, false if it hasn't been locked.
     */
    bool locked() const;

    /**
     * set the Locked flag to the give value
     */
    void setLocked(bool value);

    /**
     * @brief does the node have any content in it?
     * @return if node has any content in it
     */
    bool hasExtents();


    /**
     * @return the user-visible name of this node.
     */
    QString name() const;

    /**
     * rename the Node to the given name
     */
    void setName(QString name);

    /**
     * return the opacity of the Node. The opacity is a value between 0 and 255.
     */
    int opacity() const;

    /**
     * set the opacity of the Node to the given value. The opacity is a value between 0 and 255.
     */
    void setOpacity(int value);

    /**
     * return the Node that is the parent of the current Node, or 0 if this is the root Node.
     */
    Node* parentNode() const;

    /**
     * @brief type Krita has several types of nodes, split in layers and masks. Group
     * layers can contain other layers, any layer can contain masks.
     *
     * @return The type of the node. Valid types are:
     * <ul>
     *  <li>paintlayer
     *  <li>grouplayer
     *  <li>filelayer
     *  <li>filterlayer
     *  <li>filllayer
     *  <li>clonelayer
     *  <li>vectorlayer
     *  <li>transparencymask
     *  <li>filtermask
     *  <li>transformmask
     *  <li>selectionmask
     *  <li>colorizemask
     * </ul>
     *
     * If the Node object isn't wrapping a valid Krita layer or mask object, and
     * empty string is returned.
     */
    virtual QString type() const;

    /**
     * @brief icon
     * @return the icon associated with the layer.
     */
    QIcon icon() const;

    /**
     * Check whether the current Node is visible in the layer stack
     */
    bool visible() const;

    /**
     * Set the visibility of the current node to @param visible
     */
    void setVisible(bool visible);

    /**
     * @brief pixelData reads the given rectangle from the Node's paintable pixels, if those
     * exist, and returns it as a byte array. The pixel data starts top-left, and is ordered row-first.
     *
     * The byte array can be interpreted as follows: 8 bits images have one byte per channel,
     * and as many bytes as there are channels. 16 bits integer images have two bytes per channel,
     * representing an unsigned short. 16 bits float images have two bytes per channel, representing
     * a half, or 16 bits float. 32 bits float images have four bytes per channel, representing a
     * float.
     *
     * You can read outside the node boundaries; those pixels will be transparent black.
     *
     * The order of channels is:
     *
     * <ul>
     * <li>Integer RGBA: Blue, Green, Red, Alpha
     * <li>Float RGBA: Red, Green, Blue, Alpha
     * <li>GrayA: Gray, Alpha
     * <li>Selection: selectedness
     * <li>LabA: L, a, b, Alpha
     * <li>CMYKA: Cyan, Magenta, Yellow, Key, Alpha
     * <li>XYZA: X, Y, Z, A
     * <li>YCbCrA: Y, Cb, Cr, Alpha
     * </ul>
     *
     * The byte array is a copy of the original node data. In Python, you can use bytes, bytearray
     * and the struct module to interpret the data and construct, for instance, a Pillow Image object.
     *
     * If you read the pixeldata of a mask, a filter or generator layer, you get the selection bytes,
     * which is one channel with values in the range from 0..255.
     *
     * If you want to change the pixels of a node you can write the pixels back after manipulation
     * with setPixelData(). This will only succeed on nodes with writable pixel data, e.g not on groups
     * or file layers.
     *
     * @param x x position from where to start reading
     * @param y y position from where to start reading
     * @param w row length to read
     * @param h number of rows to read
     * @return a QByteArray with the pixel data. The byte array may be empty.

     */
    QByteArray pixelData(int x, int y, int w, int h) const;

    /**
     * @brief pixelDataAtTime a basic function to get pixeldata from an animated node at a given time.
     * @param x the position from the left to start reading.
     * @param y the position from the top to start reader
     * @param w the row length to read
     * @param h the number of rows to read
     * @param time the frame number
     * @return a QByteArray with the pixel data. The byte array may be empty.
     */
    QByteArray pixelDataAtTime(int x, int y, int w, int h, int time) const;

    /**
     * @brief projectionPixelData reads the given rectangle from the Node's projection (that is, what the node
     * looks like after all sub-Nodes (like layers in a group or masks on a layer) have been applied,
     * and returns it as a byte array. The pixel data starts top-left, and is ordered row-first.
     *
     * The byte array can be interpreted as follows: 8 bits images have one byte per channel,
     * and as many bytes as there are channels. 16 bits integer images have two bytes per channel,
     * representing an unsigned short. 16 bits float images have two bytes per channel, representing
     * a half, or 16 bits float. 32 bits float images have four bytes per channel, representing a
     * float.
     *
     * You can read outside the node boundaries; those pixels will be transparent black.
     *
     * The order of channels is:
     *
     * <ul>
     * <li>Integer RGBA: Blue, Green, Red, Alpha
     * <li>Float RGBA: Red, Green, Blue, Alpha
     * <li>GrayA: Gray, Alpha
     * <li>Selection: selectedness
     * <li>LabA: L, a, b, Alpha
     * <li>CMYKA: Cyan, Magenta, Yellow, Key, Alpha
     * <li>XYZA: X, Y, Z, A
     * <li>YCbCrA: Y, Cb, Cr, Alpha
     * </ul>
     *
     * The byte array is a copy of the original node data. In Python, you can use bytes, bytearray
     * and the struct module to interpret the data and construct, for instance, a Pillow Image object.
     *
     * If you read the projection of a mask, you get the selection bytes, which is one channel with
     * values in the range from 0..255.
     *
     * If you want to change the pixels of a node you can write the pixels back after manipulation
     * with setPixelData(). This will only succeed on nodes with writable pixel data, e.g not on groups
     * or file layers.
     *
     * @param x x position from where to start reading
     * @param y y position from where to start reading
     * @param w row length to read
     * @param h number of rows to read
     * @return a QByteArray with the pixel data. The byte array may be empty.
     */
    QByteArray projectionPixelData(int x, int y, int w, int h) const;

    /**
     * @brief setPixelData writes the given bytes, of which there must be enough, into the
     * Node, if the Node has writable pixel data:
     *
     * <ul>
     * <li>paint layer: the layer's original pixels are overwritten
     * <li>filter layer, generator layer, any mask: the embedded selection's pixels are overwritten.
     * <b>Note:</b> for these
     * </ul>
     *
     * File layers, Group layers, Clone layers cannot be written to. Calling setPixelData on
     * those layer types will silently do nothing.
     *
     * @param value the byte array representing the pixels. There must be enough bytes available.
     * Krita will take the raw pointer from the QByteArray and start reading, not stopping before
     * (number of channels * size of channel * w * h) bytes are read.
     *
     * @param x the x position to start writing from
     * @param y the y position to start writing from
     * @param w the width of each row
     * @param h the number of rows to write
     */
    void setPixelData(QByteArray value, int x, int y, int w, int h);

    /**
     * @brief bounds return the exact bounds of the node's paint device
     * @return the bounds, or an empty QRect if the node has no paint device or is empty.
     */
    QRect bounds() const;

    /**
     *  move the pixels to the given x, y location in the image coordinate space.
     */
    void move(int x, int y);

    /**
     * @brief position returns the position of the paint device of this node. The position is
     * always 0,0 unless the layer has been moved. If you want to know the topleft position of
     * the rectangle around the actual non-transparent pixels in the node, use bounds().
     * @return the top-left position of the node
     */
    QPoint position() const;

    /**
     * @brief remove removes this node from its parent image.
     */
    bool remove();

    /**
     * @brief duplicate returns a full copy of the current node. The node is not inserted in the graphic
     * @return a valid Node object or 0 if the node couldn't be duplicated.
     */
    Node* duplicate();

    /**
     * @brief save exports the given node with this filename. The extension of the filename determines the filetype.
     * @param filename the filename including extension
     * @param xRes the horizontal resolution in pixels per pt (there are 72 pts in an inch)
     * @param yRes the horizontal resolution in pixels per pt (there are 72 pts in an inch)
     * @return true if saving succeeded, false if it failed.
     */
    bool save(const QString &filename, double xRes, double yRes);

    /**
     * @brief mergeDown merges the given node with the first visible node underneath this node in the layerstack.
     * This will drop all per-layer metadata.
     */
    Node *mergeDown();

    /**
     * @brief scaleNode
     * @param width
     * @param height
     * @param strategy the scaling strategy. There's several ones amongst these that aren't available in the regular UI.
     * <ul>
     * <li>Hermite</li>
     * <li>Bicubic - Adds pixels using the color of surrounding pixels. Produces smoother tonal gradations than Bilinear.</li>
     * <li>Box - Replicate pixels in the image. Preserves all the original detail, but can produce jagged effects.</li>
     * <li>Bilinear - Adds pixels averaging the color values of surrounding pixels. Produces medium quality results when the image is scaled from half to two times the original size.</li>
     * <li>Bell</li>
     * <li>BSpline</li>
     * <li>Lanczos3 - Offers similar results than Bicubic, but maybe a little bit sharper. Can produce light and dark halos along strong edges.</li>
     * <li>Mitchell</li>
     * </ul>
     */
    void scaleNode(QPointF origin, int width, int height, QString strategy);

    /**
     * @brief rotateNode rotate this layer by the given radians.
     * @param radians amount the layer should be rotated in, in radians.
     */
    void rotateNode(double radians);

    /**
     * @brief cropNode crop this layer.
     * @param x the left edge of the cropping rectangle.
     * @param y the top edge of the cropping rectangle
     * @param w the right edge of the cropping rectangle
     * @param h the bottom edge of the cropping rectangle
     */
    void cropNode(int x, int y, int w, int h);

    /**
     * @brief shearNode perform a shear operation on this node.
     * @param angleX the X-angle in degrees to shear by
     * @param angleY the Y-angle in degrees to shear by
     */
    void shearNode(double angleX, double angleY);

    /**
     * @brief thumbnail create a thumbnail of the given dimensions. The thumbnail is sized according
     * to the layer dimensions, not the image dimensions. If the requested size is too big a null
     * QImage is created. If the current node cannot generate a thumbnail, a transparent QImage of the
     * requested size is generated.
     * @return a QImage representing the layer contents.
     */
    QImage thumbnail(int w, int h);

private:

    friend class Filter;
    friend class Document;
    friend class Selection;
    friend class GroupLayer;
    friend class FileLayer;
    friend class FilterLayer;
    friend class FillLayer;
    friend class VectorLayer;
    friend class FilterMask;
    friend class SelectionMask;
    /**
     * @brief paintDevice gives access to the internal paint device of this Node
     * @return the paintdevice or 0 if the node does not have an editable paint device.
     */
    KisPaintDeviceSP paintDevice() const;
    KisImageSP image() const;
    KisNodeSP node() const;

    struct Private;
    Private *const d;

};

#endif // LIBKIS_NODE_H
