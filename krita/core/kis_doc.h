/*
 *  kis_doc.h - part of Krayon
 *
 *  Copyright (c) 1999-2000 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2001 Toshitaka Fujioka  <fujioka@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __kis_doc_h__
#define __kis_doc_h__

#include <qstring.h>
#include <qptrlist.h>
#include <qstringlist.h>

#include <kcommand.h>
#include <klocale.h>
#include <ksharedptr.h>

#include <koDocument.h>

#include "kis_image.h"
#include "kis_framebuffer.h"
#include "kis_global.h"
#include "kis_selection.h"
#include "kis_view.h"

class Magick::Image;
class DCOPObject;

/*
 * A KisDoc can hold multiple KisImages.
 *
 * KisDoc -> currentImg() returns a Pointer to the currently active KisImage.
 */

class KisDoc : public KoDocument
{
	typedef KoDocument super;
	Q_OBJECT

public:
	KisDoc(QWidget *parentWidget = 0, const char *widgetName = 0, QObject* parent = 0, const char* name = 0, bool singleViewMode = false);
	virtual ~KisDoc();

	/*
	 * Reimplemented from KoDocument.
	 * See koDocument.h.
	 */

	virtual QCString mimeType() const;
	virtual bool initDoc();
	virtual QDomDocument saveXML();
	virtual bool loadXML(QIODevice *, const QDomDocument& doc);
	virtual bool completeLoading(KoStore *store);
	virtual bool completeSaving(KoStore*);
	virtual DCOPObject* dcopObject();

	virtual void paintContent(QPainter& painter, const QRect& rect, bool transparent = false, double zoomX = 1.0, double zoomY = 1.0);

	void addCommand(KCommand *cmd);
	int undoLimit () const;
	void setUndoLimit(int limit);
	int redoLimit() const;
	void setRedoLimit(int limit);

	/*
	 * Use QPainter p to paint a rectangular are of the current image.
	 */
	void paintPixmap( QPainter* p, QRect area );

	/*
	 * Create new KisImage, add it to our KisImage list and make it the current Image.
	 */
	KisImageSP newImage(const QString& name, int width, int height, cMode cm = cm_RGBA, uchar bitDepth = 8);

	void addImage(KisImageSP img);

	/*
	 * Remove img from our list and delete it.
	 */
	void removeImage(KisImageSP img);

	/*
	 * Return apointer to the current view.
	 */
	KisView* currentView();
	const KisView* currentView() const;


	/*
	 * Return apointer to the current image.
	 */
	KisImageSP currentImg() const;
	KisImageSP imageNum( unsigned int _num );


	/*
	 * Return the name of the current image.
	 */
	QString currentImgName();

	/*
	 * Make img the current image.
	 */
	void setCurrentImage(KisImageSP img);

	/*
	 * Unset the current image.
	 */

	void unsetCurrentImage();

	/*
	 * Does the doc contain any images?
	 */
	bool isEmpty() const;

	/*
	 * Return a list of image names.
	 */
	QStringList images();

	/*
	 * Rename an image
	 */
	void renameImage(const QString& oldName, const QString& newName);

	/*
	 *  save current image as Qt image (standard image formats)
	 */
	bool saveAsQtImage(const QString& file, bool wholeImage);

	bool MagickImageToLayer(const Magick::Image& img, KisView *view);

	/*
	 *  needs to go in kis_framebuffer.cc
	 */
	bool QtImageToLayer(QImage *qimage, KisView *pView);

	/*
	 *  copy rectangular area of layer to Qt Image
	 */
	bool LayerToQtImage(QImage *qimage, const QRect& clipRect);

	/*
	 *  set selection or clip rectangle for the document
	 */
	bool setClipImage();

	/*
	 *  get selection or clip image for the document
	 */
	QImage *getClipImage() { return m_pClipImage; }

	/*
	 *  delete clip image for the document
	 */
	void removeClipImage();

	/*
	 *  get currrent selection for document
	 */
	KisSelection *getSelection() { return m_pSelection; }

	/*
	 *  set selection for document
	 */
	void setSelection(const QRect& r);

	/*
	 *  clear selection for document -
	 */
	void clearSelection();

	/*
	 *  does the document have a selection ?
	 */
	bool hasSelection();

	/*
	 *  get FrameBuffer
	 */
	KisFrameBuffer *frameBuffer() { return m_pFrameBuffer; }

	QRect getImageRect();

	void setImage(const QString& imageName); // for print, save file and load file.

	ktvector getTools() const;
	void setTools(const ktvector& tools);
	inline void setUndo(bool undo);

	void setCanvasCursor(const QCursor& cursor);

#if 0
    /*
     * Gradients settings
     */
    struct GradientsSettings {
        GradientsSettings() {
            opacity = 100;
            offset = 0;
            mode = i18n( "Normal" );
            blend = i18n( "FG to BG (RGB)" );
            gradient = i18n( "Vertical" );
            repeat = i18n( "None" );
        }

        uint opacity, offset;
        QString mode, blend, gradient, repeat;
    };

    GradientsSettings getGradientsToolSettings() const { return gradientsSettings; }
    void setGradientsSettings( GradientsSettings s );
#endif

public slots:
	void slotImageUpdated();
	void slotImageUpdated(const QRect& rect);
	void slotLayersUpdated();

	bool slotNewImage();
	void setCurrentImage(const QString& name);
	void slotRemoveImage(const QString& name);

	void slotDocumentRestored();
	void slotCommandExecuted();

signals:
	void docUpdated();
	void docUpdated(const QRect& rect);
	void layersUpdated();
	void imageListUpdated();

protected:

	/* reimplemented from koDocument - a document can have multiple
	   views of the same data */
	virtual KoView* createViewInstance(QWidget *parent, const char *name);

	QDomElement saveImages(QDomDocument& doc);
	QDomElement saveLayers(QDomDocument& doc, KisImageSP img);
	QDomElement saveChannels(QDomDocument& doc, KisLayer* lay);
	QDomElement saveToolSettings(QDomDocument& doc) const;

	bool loadImages(QDomElement& elem);
	bool loadLayers(QDomElement& elem, KisImageSP img);
	void loadChannels(QDomElement& elem, KisLayerSP lay);
	void loadToolSettings(QDomElement& elem);
	bool loadImgSettings(QDomElement& elem);

private:
	bool m_undo;
	/* undo/redo */
	KCommandHistory *m_cmdHistory;

	/* list of images for the document - each document can have multiple
	   images and each image must have at least one layer. however, a document
	   can only have one current image, which is what is loaded and saved -
	   the permanent data associated with it. This coresponds to an
	   image, but that image is interchangeable */
	KisImageSPLst m_images;

	KisView *m_currentView;
	KisImageSP m_currentImg;
	QImage *m_pClipImage;

	KisSelection *m_pSelection;
	KisFrameBuffer *m_pFrameBuffer;

	ktvector m_tools;
	DCOPObject *dcop;
};

void KisDoc::setUndo(bool undo)
{
	m_undo = undo;
}

#endif // __kis_doc_h__

