/*
 *  Copyright (c) 1999-2000 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2001 Toshitaka Fujioka  <fujioka@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2007 Boudewijn Rempt <boud@valdyas.org<
 *  Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_DOC_2_H_
#define KIS_DOC_2_H_

#include <kis_debug.h>

#include <KoDocument.h>

#include "kis_types.h"

#include <krita_export.h>

class QString;

class KoColorSpace;
class KoColor;
class KoShape;

class KoShapeControllerBase;
class KisView2;
class KisChildDoc;
class KisUndoAdapter;
class KisNodeModel;
/**
 * The class that represents a Krita document containing content and
   settings.

   KisDoc2 also keeps track of the correspondence between the layer
   structure of the KisImage and the shape tree that is used by the
   tools.


   XXX: File format TODOs

   * load/save the channel flags setting for layers

 */
class KRITAUI_EXPORT KisDoc2 : public KoDocument
{

    Q_OBJECT

public:
    KisDoc2(QWidget *parentWidget = 0, QObject* parent = 0, bool singleViewMode = true);
    virtual ~KisDoc2();

public:
    // Overide KoDocument
    virtual bool wantExportConfirmation() const {
        return false;
    }
    virtual bool completeLoading(KoStore *store);
    virtual bool completeSaving(KoStore*);
    virtual bool loadOdf(KoOdfReadStore & odfStore);
    virtual bool saveOdf(SavingContext &documentContext);
    virtual bool loadXML(const KoXmlDocument& doc, KoStore* store);
    virtual QByteArray mimeType() const;
    virtual QList<KoDocument::CustomDocumentWidgetItem> createCustomDocumentWidgets(QWidget *parent);

    /**
     * Draw the image embedded in another KOffice document
     */
    virtual void paintContent(QPainter& painter, const QRect& rect);

    /// Generate a scaled-down pixmap of the image projection that fits in size
    virtual QPixmap generatePreview(const QSize& size);

    virtual QDomDocument saveXML();

    virtual void setUndo(bool undo);
    virtual bool undo() const;

public slots:

    /**
     * Initialize an empty document using default values
     * @since 1.5
     */
    virtual void initEmpty();

public:

    /**
     * Create a new image that has this document as a parent and
     * replace the current image with this image.
     */
    bool newImage(const QString& name, qint32 width, qint32 height, const KoColorSpace * cs, const KoColor &bgColor, const QString &imageDescription, const double imageResolution);

    /**
     * Create a new image that has this document as a parent and
     * replace the current image with this image.
     */
    KisImageWSP newImage(const QString& name, qint32 width, qint32 height, const KoColorSpace * colorspace);

    KisImageWSP image() const;

    /**
     * Makes an otherwise empty document ready for import/export
     */
    void prepareForImport();

    /**
     * Set the current image to the specified image and turn undo on.
     */
    void setCurrentImage(KisImageWSP image);

    KisUndoAdapter * undoAdapter() const;

    /**
     * The shape controller matches internal krita image layers with
     * the flake shape hierarchy.
     */
    KoShapeControllerBase * shapeController() const;

    KoShape * shapeForNode(KisNodeSP layer) const;

    /**
     * Add a node to the shape controller
     */
    KoShape * addShape(const KisNodeSP node);

    /**
     * The layer model provides a light-weight Qt model-view
     * compatible model on the internal Krita image layer hierarchy.
     */
    KisNodeModel * nodeModel() const;

signals:

    void sigLoadingFinished();

public:

    // Overide KoDocument
    virtual KoView* createViewInstance(QWidget *parent);

protected slots:

    // Overide KoDocument
    virtual void openExistingFile(const KUrl& url);
    virtual void openTemplate(const KUrl& url);

private slots:

    void slotIOProgress(qint8 percentage);
    void undoIndexChanged(int idx);

private:

    bool init();

    void setIOSteps(qint32 nsteps);
    void IOCompletedStep();
    void IODone();

private:

    class KisDocPrivate;
    KisDocPrivate * const m_d;

};

#endif // KIS_DOC_H_
