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
#ifndef LIBKIS_DOCUMENT_H
#define LIBKIS_DOCUMENT_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

#include "GroupLayer.h"
#include "CloneLayer.h"
#include "FileLayer.h"
#include "FilterLayer.h"
#include "FillLayer.h"
#include "VectorLayer.h"
#include "FilterMask.h"
#include "SelectionMask.h"

class KisDocument;

/**
 * The Document class encapsulates a Krita Document/Image. A Krita document is an Image with
 * a filename. Libkis does not differentiate between a document and an image, like Krita does
 * internally.
 */
class KRITALIBKIS_EXPORT Document : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Document)

public:
    explicit Document(KisDocument *document, QObject *parent = 0);
    ~Document() override;

    bool operator==(const Document &other) const;
    bool operator!=(const Document &other) const;

    /**
     * @brief horizontalGuides
     * The horizontal guides.
     * @return a list of the horizontal positions of guides.
     */
    QList<qreal> horizontalGuides() const;
    /**
     * @brief verticalGuides
     * The vertical guide lines.
     * @return a list of vertical guides.
     */
    QList<qreal> verticalGuides() const;

    /**
     * @brief guidesVisible
     * Returns guide visibility.
     * @return whether the guides are visible.
     */
    bool guidesVisible() const;
    /**
     * @brief guidesLocked
     * Returns guide lockedness.
     * @return whether the guides are locked.
     */
    bool guidesLocked() const;

public Q_SLOTS:

    /**
     * @brief clone create a shallow clone of this document.
     * @return a new Document that should be identical to this one in every respect.
     */
    Document *clone() const;

    /**
     * Batchmode means that no actions on the document should show dialogs or popups.
     * @return true if the document is in batchmode.
     */
    bool batchmode() const;

    /**
     * Set batchmode to @param value. If batchmode is true, then there should be no popups
     * or dialogs shown to the user.
     */
    void setBatchmode(bool value);

    /**
     * @brief activeNode retrieve the node that is currently active in the currently active window
     * @return the active node. If there is no active window, the first child node is returned.
     */
    Node* activeNode() const;

    /**
     * @brief setActiveNode make the given node active in the currently active view and window
     * @param value the node to make active.
     */
    void setActiveNode(Node* value);

    /**
     * @brief toplevelNodes return a list with all top level nodes in the image graph
     */
    QList<Node*> topLevelNodes() const;

    /**
     * @brief nodeByName searches the node tree for a node with the given name and returns it
     * @param name the name of the node
     * @return the first node with the given name or 0 if no node is found
     */
    Node *nodeByName(const QString &name) const;

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
     * @return false if the colorProfile name does not correspond to to a registered profile or if assigning
     * the profile failed.
     */
    bool setColorProfile(const QString &colorProfile);

    /**
     * @brief setColorSpace convert the nodes and the image to the given colorspace. The conversion is
     * done with Perceptual as intent, High Quality and No LCMS Optimizations as flags and no blackpoint
     * compensation.
     *
     * @param colorModel A string describing the color model of the image:
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
     * @return false the combination of these arguments does not correspond to a colorspace.
     */
    bool setColorSpace(const QString &colorModel, const QString &colorDepth, const QString &colorProfile);

    /**
     * @brief backgroundColor returns the current background color of the document. The color will
     * also include the opacity.
     *
     * @return QColor
     */
    QColor backgroundColor();

    /**
     * @brief setBackgroundColor sets the background color of the document. It will trigger a projection
     * update.
     *
     * @param color A QColor. The color will be converted from sRGB.
     * @return bool
     */
    bool setBackgroundColor(const QColor &color);

    /**
     * @brief documentInfo creates and XML document representing document and author information.
     * @return a string containing a valid XML document with the right information about the document
     * and author. The DTD can be found here:
     *
     * https://phabricator.kde.org/source/krita/browse/master/krita/dtd/
     *
     * @code
     * <?xml version="1.0" encoding="UTF-8"?>
     * <!DOCTYPE document-info PUBLIC '-//KDE//DTD document-info 1.1//EN' 'http://www.calligra.org/DTD/document-info-1.1.dtd'>
     * <document-info xmlns="http://www.calligra.org/DTD/document-info">
     * <about>
     *  <title>My Document</title>
     *   <description></description>
     *   <subject></subject>
     *   <abstract><![CDATA[]]></abstract>
     *   <keyword></keyword>
     *   <initial-creator>Unknown</initial-creator>
     *   <editing-cycles>1</editing-cycles>
     *   <editing-time>35</editing-time>
     *   <date>2017-02-27T20:15:09</date>
     *   <creation-date>2017-02-27T20:14:33</creation-date>
     *   <language></language>
     *  </about>
     *  <author>
     *   <full-name>Boudewijn Rempt</full-name>
     *   <initial></initial>
     *   <author-title></author-title>
     *   <email></email>
     *   <telephone></telephone>
     *   <telephone-work></telephone-work>
     *   <fax></fax>
     *   <country></country>
     *   <postal-code></postal-code>
     *   <city></city>
     *   <street></street>
     *   <position></position>
     *   <company></company>
     *  </author>
     * </document-info>
     * @endcode
     *
     */
    QString documentInfo() const;

    /**
     * @brief setDocumentInfo set the Document information to the information contained in document
     * @param document A string containing a valid XML document that conforms to the document-info DTD
     * that can be found here:
     *
     * https://phabricator.kde.org/source/krita/browse/master/krita/dtd/
     */
    void setDocumentInfo(const QString &document);

    /**
     * @return the full path to the document, if it has been set.
     */
    QString fileName() const;

    /**
     * @brief setFileName set the full path of the document to @param value
     */
    void setFileName(QString value);

    /**
     * @return the height of the image in pixels
     */
    int height() const;

    /**
     * @brief setHeight resize the document to @param value height. This is a canvas resize, not a scale.
     */
    void setHeight(int value);

    /**
     * @return the name of the document. This is the title field in the @see documentInfo
     */
    QString name() const;

    /**
     * @brief setName sets the name of the document to @param value. This is the title field in the @see documentInfo
     */
    void setName(QString value);

    /**
     * @return the resolution in pixels per inch
     */
    int resolution() const;
    /**
     * @brief setResolution set the resolution of the image; this does not scale the image
     * @param value the resolution in pixels per inch
     */
    void setResolution(int value);

    /**
     * @brief rootNode the root node is the invisible group layer that contains the entire node
     * hierarchy.
     * @return the root of the image
     */
    Node* rootNode() const;

    /**
     * @brief selection Create a Selection object around the global selection, if there is one.
     * @return the global selection or None if there is no global selection.
     */
    Selection* selection() const;

    /**
     * @brief setSelection set or replace the global selection
     * @param value a valid selection object.
     */
    void setSelection(Selection* value);

    /**
     * @return the width of the image in pixels.
     */
    int width() const;

    /**
     * @brief setWidth resize the document to @param value width. This is a canvas resize, not a scale.
     */
    void setWidth(int value);

    /**
     * @return the left edge of the canvas in pixels.
     */
    int xOffset() const;

    /**
     * @brief setXOffset sets the left edge of the canvas to @param x.
     */
    void setXOffset(int x);

    /**
     * @return the top edge of the canvas in pixels.
     */
    int yOffset() const;

    /**
     * @brief setYOffset sets the top edge of the canvas to @param y.
     */
    void setYOffset(int y);

    /**
     * @return xRes the horizontal resolution of the image in pixels per pt (there are 72 pts to an inch)
     */

    double xRes() const;

    /**
     * @brief setXRes set the horizontal resolution of the image to xRes in pixels per pt. (there are 72 pts to an inch)
     */
    void setXRes(double xRes) const;

    /**
     * @return yRes the vertical resolution of the image in pixels per pt (there are 72 pts to an inch)
     */
    double yRes() const;

    /**
     * @brief setYRes set the vertical resolution of the image to yRes in pixels per pt. (there are 72 pts to an inch)
     */
    void setYRes(double yRes) const;

    /**
     * @brief pixelData reads the given rectangle from the image projection and returns it as a byte
     * array. The pixel data starts top-left, and is ordered row-first.
     *
     * The byte array can be interpreted as follows: 8 bits images have one byte per channel,
     * and as many bytes as there are channels. 16 bits integer images have two bytes per channel,
     * representing an unsigned short. 16 bits float images have two bytes per channel, representing
     * a half, or 16 bits float. 32 bits float images have four bytes per channel, representing a
     * float.
     *
     * You can read outside the image boundaries; those pixels will be transparent black.
     *
     * The order of channels is:
     *
     * <ul>
     * <li>Integer RGBA: Blue, Green, Red, Alpha
     * <li>Float RGBA: Red, Green, Blue, Alpha
     * <li>LabA: L, a, b, Alpha
     * <li>CMYKA: Cyan, Magenta, Yellow, Key, Alpha
     * <li>XYZA: X, Y, Z, A
     * <li>YCbCrA: Y, Cb, Cr, Alpha
     * </ul>
     *
     * The byte array is a copy of the original image data. In Python, you can use bytes, bytearray
     * and the struct module to interpret the data and construct, for instance, a Pillow Image object.
     *
     * @param x x position from where to start reading
     * @param y y position from where to start reading
     * @param w row length to read
     * @param h number of rows to read
     * @return a QByteArray with the pixel data. The byte array may be empty.
     */
    QByteArray pixelData(int x, int y, int w, int h) const;

    /**
     * @brief close Close the document: remove it from Krita's internal list of documents and
     * close all views. If the document is modified, you should save it first. There will be
     * no prompt for saving.
     *
     * After closing the document it becomes invalid.
     *
     * @return true if the document is closed.
     */
    bool close();

    /**
     * @brief crop the image to rectangle described by @param x, @param y,
     * @param w and @param h
     */
    void crop(int x, int y, int w, int h);

    /**
     * @brief exportImage export the image, without changing its URL to the given path.
     * @param filename the full path to which the image is to be saved
     * @param exportConfiguration a configuration object appropriate to the file format.
     * An InfoObject will used to that configuration.
     *
     * The supported formats have specific configurations that must be used when in
     * batchmode. They are described below:
     *
     *\b png
     * <ul>
     * <li>alpha: bool (True or False)
     * <li>compression: int (1 to 9)
     * <li>forceSRGB: bool (True or False)
     * <li>indexed: bool (True or False)
     * <li>interlaced: bool (True or False)
     * <li>saveSRGBProfile: bool (True or False)
     * <li>transparencyFillcolor: rgb (Ex:[255,255,255])
     * </ul>
     *
     *\b jpeg
     * <ul>
     * <li>baseline: bool (True or False)
     * <li>exif: bool (True or False)
     * <li>filters: bool (['ToolInfo', 'Anonymizer'])
     * <li>forceSRGB: bool (True or False)
     * <li>iptc: bool (True or False)
     * <li>is_sRGB: bool (True or False)
     * <li>optimize: bool (True or False)
     * <li>progressive: bool (True or False)
     * <li>quality: int (0 to 100)
     * <li>saveProfile: bool (True or False)
     * <li>smoothing: int (0 to 100)
     * <li>subsampling: int (0 to 3)
     * <li>transparencyFillcolor: rgb (Ex:[255,255,255])
     * <li>xmp: bool (True or False)
     * </ul>
     * @return true if the export succeeded, false if it failed.
     */
    bool exportImage(const QString &filename, const InfoObject &exportConfiguration);

    /**
     * @brief flatten all layers in the image
     */
    void flatten();

    /**
     * @brief resizeImage resizes the canvas to the given left edge, top edge, width and height.
     * Note: This doesn't scale, use scale image for that.
     * @param x the new left edge
     * @param y the new top edge
     * @param w the new width
     * @param h the new height
     */
    void resizeImage(int x, int y, int w, int h);

    /**
    * @brief scaleImage
    * @param w the new width
    * @param h the new height
    * @param xres the new xres
    * @param yres the new yres
    * @param strategy the scaling strategy. There's several ones amongst these that aren't available in the regular UI.
    * The list of filters is extensible and can be retrieved with Krita::filter
    * <ul>
    * <li>Hermite</li>
    * <li>Bicubic - Adds pixels using the color of surrounding pixels. Produces smoother tonal gradations than Bilinear.</li>
    * <li>Box - Replicate pixels in the image. Preserves all the original detail, but can produce jagged effects.</li>
    * <li>Bilinear - Adds pixels averaging the color values of surrounding pixels. Produces medium quality results when the image is scaled from half to two times the original size.</li>
    * <li>Bell</li>
    * <li>BSpline</li>
    * <li>Kanczos3 - Offers similar results than Bicubic, but maybe a little bit sharper. Can produce light and dark halos along strong edges.</li>
    * <li>Mitchell</li>
    * </ul>
    */
   void scaleImage(int w, int h, int xres, int yres, QString strategy);

   /**
    * @brief rotateImage
    * Rotate the image by the given radians.
    * @param radians the amount you wish to rotate the image in radians
    */
   void rotateImage(double radians);

   /**
    * @brief shearImage shear the whole image.
    * @param angleX the X-angle in degrees to shear by
    * @param angleY the Y-angle in degrees to shear by
    */
   void shearImage(double angleX, double angleY);

    /**
     * @brief save the image to its currently set path. The modified flag of the
     * document will be reset
     * @return true if saving succeeded, false otherwise.
     */
    bool save();

    /**
     * @brief saveAs save the document under the @param filename. The document's
     * filename will be reset to @param filename.
     * @param filename the new filename (full path) for the document
     * @return true if saving succeeded, false otherwise.
     */
    bool saveAs(const QString &filename);

    /**
     * @brief createNode create a new node of the given type. The node is not added
     * to the node hierarchy; you need to do that by finding the right parent node,
     * getting its list of child nodes and adding the node in the right place, then
     * calling Node::SetChildNodes
     *
     * @param name The name of the node
     *
     * @param nodeType The type of the node. Valid types are:
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
     * </ul>
     *
     * When relevant, the new Node will have the colorspace of the image by default;
     * that can be changed with Node::setColorSpace.
     *
     * The settings and selections for relevant layer and mask types can also be set
     * after the Node has been created.
     *
@code
d = Application.createDocument(1000, 1000, "Test", "RGBA", "U8", "", 120.0)
root = d.rootNode();
print(root.childNodes())
l2 = d.createNode("layer2", "paintLayer")
print(l2)
root.addChildNode(l2, None)
print(root.childNodes())
@endcode
     *
     *
     * @return the new Node.
     */
    Node* createNode(const QString &name, const QString &nodeType);
    /**
     * @brief createGroupLayer
     * Returns a grouplayer object. Grouplayers are nodes that can have
     * other layers as children and have the passthrough mode.
     * @param name the name of the layer.
     * @return a GroupLayer object.
     */
    GroupLayer* createGroupLayer(const QString &name);
    /**
     * @brief createFileLayer returns a layer that shows an external image.
     * @param name name of the file layer.
     * @param fileName the absolute filename of the file referenced. Symlinks will be resolved.
     * @param scalingMethod how the dimensions of the file are interpreted
     *        can be either "None", "ImageToSize" or "ImageToPPI"
     * @return a FileLayer
     */
    FileLayer* createFileLayer(const QString &name, const QString fileName, const QString scalingMethod);

    /**
     * @brief createFilterLayer creates a filter layer, which is a layer that represents a filter
     * applied non-destructively.
     * @param name name of the filterLayer
     * @param filter the filter that this filter layer will us.
     * @param selection the selection.
     * @return a filter layer object.
     */
    FilterLayer* createFilterLayer(const QString &name, Filter &filter, Selection &selection);

    /**
     * @brief createFillLayer creates a fill layer object, which is a layer
     * @param name
     * @param generatorName - name of the generation filter.
     * @param configuration - the configuration for the generation filter.
     * @param selection - the selection.
     * @return a filllayer object.
     *
     * @code
     * from krita import *
     * d = Krita.instance().activeDocument()
     * i = InfoObject();
     * i.setProperty("pattern", "Cross01.pat")
     * s = Selection();
     * s.select(0, 0, d.width(), d.height(), 255)
     * n = d.createFillLayer("test", "pattern", i, s)
     * r = d.rootNode();
     * c = r.childNodes();
     * r.addChildNode(n, c[0])
     * d.refreshProjection()
     * @endcode
     */
    FillLayer* createFillLayer(const QString &name, const QString generatorName, InfoObject &configuration, Selection &selection);

    /**
     * @brief createCloneLayer
     * @param name
     * @param source
     * @return
     */
    CloneLayer* createCloneLayer(const QString &name, const Node* source);

    /**
     * @brief createVectorLayer
     * Creates a vector layer that can contain vector shapes.
     * @param name the name of this layer.
     * @return a VectorLayer.
     */
    VectorLayer* createVectorLayer(const QString &name);

    /**
     * @brief createFilterMask
     * Creates a filter mask object that much like a filterlayer can apply a filter non-destructively.
     * @param name the name of the layer.
     * @param filter the filter assigned.
     * @return a FilterMask
     */
    FilterMask* createFilterMask(const QString &name, Filter &filter);

    /**
     * @brief createSelectionMask
     * Creates a selection mask, which can be used to store selections.
     * @param name - the name of the layer.
     * @return a SelectionMask
     */
    SelectionMask* createSelectionMask(const QString &name);


    /**
     * @brief projection creates a QImage from the rendered image or
     * a cutout rectangle.
     */
    QImage projection(int x = 0, int y = 0, int w = 0, int h = 0) const;

    /**
     * @brief thumbnail create a thumbnail of the given dimensions.
     *
     * If the requested size is too big a null QImage is created.
     *
     * @return a QImage representing the layer contents.
     */
    QImage thumbnail(int w, int h) const;


    /**
     * Why this should be used, When it should be used, How it should be used,
     * and warnings about when not.
     */
    void lock();

    /**
     * Why this should be used, When it should be used, How it should be used,
     * and warnings about when not.
     */
    void unlock();

    /**
     * Why this should be used, When it should be used, How it should be used,
     * and warnings about when not.
     */
    void waitForDone();

    /**
     * Why this should be used, When it should be used, How it should be used,
     * and warnings about when not.
     */
    bool tryBarrierLock();

    /**
     * Why this should be used, When it should be used, How it should be used,
     * and warnings about when not.
     */
    bool isIdle();

    /**
     * Starts a synchronous recomposition of the projection: everything will
     * wait until the image is fully recomputed.
     */
    void refreshProjection();
    /**
     * @brief setHorizontalGuides
     * replace all existing horizontal guides with the entries in the list.
     * @param list a list of floats containing the new guides.
     */
    void setHorizontalGuides(const QList<qreal> &lines);
    /**
     * @brief setVerticalGuides
     * replace all existing horizontal guides with the entries in the list.
     * @param list a list of floats containing the new guides.
     */
    void setVerticalGuides(const QList<qreal> &lines);

    /**
     * @brief setGuidesVisible
     * set guides visible on this document.
     * @param visible whether or not the guides are visible.
     */
    void setGuidesVisible(bool visible);

    /**
     * @brief setGuidesLocked
     * set guides locked on this document
     * @param locked whether or not to lock the guides on this document.
     */
    void setGuidesLocked(bool locked);

    /**
     * @brief modified returns true if the document has unsaved modifications.
     */
    bool modified() const;

    /**
     * @brief bounds return the bounds of the image
     * @return the bounds
     */
    QRect bounds() const;

private:

    friend class Krita;
    friend class Window;
    friend class Filter;
    QPointer<KisDocument> document() const;


private:
    struct Private;
    Private *const d;

};

#endif // LIBKIS_DOCUMENT_H
