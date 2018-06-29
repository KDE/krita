/*
 *  Copyright (c) 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kis_debug.h>

#include <klocalizedstring.h>
#include <KoCanvasResourceManager.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServerObserver.h>
#include <KoResourceServerAdapter.h>
#include <KoCanvasBase.h>
#include <KoColor.h>
#include <resources/KoGamutMask.h>
#include <kis_icon_utils.h>
#include <KisPart.h>
#include <kis_shape_layer.h>
#include <kis_types.h>
#include <KisDocument.h>
#include <kis_node_selection_adapter.h>
#include <kis_group_layer.h>
#include <KisView.h>
#include <KoResourceItemChooser.h>

#include <QWidget>
#include <QMenu>
#include <QButtonGroup>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QFileInfo>
#include <QMessageBox>

#include "gamutmask_dock.h"
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>

#include "ui_wdgGamutMaskChooser.h"

class KisMainWindow;

struct GamutMaskChooserUI: public QWidget, public Ui_wdgGamutMaskChooser
{
    GamutMaskChooserUI() {
        setupUi(this);
    }
};


GamutMaskDock::GamutMaskDock()
    : QDockWidget(i18n("Gamut Masks"))
    , m_resourceProvider(0)
    , m_maskDocument(nullptr)
    , m_view(nullptr)
    , m_selectedMask(nullptr)
{
    m_dockerUI    = new GamutMaskChooserUI();

    m_dockerUI->bnMaskEditor->setIcon(KisIconUtils::loadIcon("dirty-preset"));
    m_dockerUI->bnMaskSet->setIcon(KisIconUtils::loadIcon("dialog-ok"));
    m_dockerUI->bnMaskDelete->setIcon(KisIconUtils::loadIcon("deletelayer"));
    m_dockerUI->maskPropertiesBox->setVisible(false);

    m_dockerUI->bnSaveMaskNew->setIcon(KisIconUtils::loadIcon("list-add"));
    m_dockerUI->bnSaveMask->setIcon(KisIconUtils::loadIcon("document-save"));
    m_dockerUI->bnCancelMaskEdit->setIcon(KisIconUtils::loadIcon("dialog-cancel"));
    m_dockerUI->bnPreviewMask->setIcon(KisIconUtils::loadIcon("visible"));

    QRegularExpression maskTitleRegex("^[-_\\sA-Za-z0-9]+$");
    QRegularExpressionValidator* m_maskTitleValidator = new QRegularExpressionValidator(maskTitleRegex, this);
    m_dockerUI->maskTitleEdit->setValidator(m_maskTitleValidator);

    KoResourceServer<KoGamutMask>* rServer = KoResourceServerProvider::instance()->gamutMaskServer();
    rServer->addObserver(this);

    // gamut mask connections
    connect(m_dockerUI->bnSaveMask        , SIGNAL(clicked())                        , SLOT(slotGamutMaskSave()));
    connect(m_dockerUI->bnCancelMaskEdit        , SIGNAL(clicked())                        , SLOT(slotGamutMaskCancelEdit()));
    connect(m_dockerUI->bnPreviewMask        , SIGNAL(clicked())                        , SLOT(slotGamutMaskPreview()));

    connect(m_dockerUI->bnMaskEditor          , SIGNAL(clicked())               , SLOT(slotGamutMaskEdit()));
    connect(m_dockerUI->maskChooser, SIGNAL(sigGamutMaskSelected(KoGamutMask*)), SLOT(slotGamutMaskSelected(KoGamutMask*)));
    connect(m_dockerUI->bnSaveMaskNew          , SIGNAL(clicked())               , SLOT(slotGamutMaskSaveNew()));
    connect(m_dockerUI->bnMaskSet          , SIGNAL(clicked())               , SLOT(slotGamutMaskSet()));
    connect(m_dockerUI->bnMaskDelete          , SIGNAL(clicked())               , SLOT(slotGamutMaskDelete()));

    setWidget(m_dockerUI);
}

GamutMaskDock::~GamutMaskDock()
{
    KoResourceServer<KoGamutMask>* rServer = KoResourceServerProvider::instance()->gamutMaskServer();
    rServer->removeObserver(this);
}

void GamutMaskDock::setMainWindow(KisViewManager* kisview)
{
    m_resourceProvider = kisview->resourceProvider();

    m_selectedMask = m_resourceProvider->currentGamutMask();

    connect(this, SIGNAL(sigGamutMaskSet(KoGamutMask*)), m_resourceProvider, SLOT(slotGamutMaskActivated(KoGamutMask*)));
    connect(this, SIGNAL(sigGamutMaskChanged(KoGamutMask*)), m_resourceProvider, SLOT(slotGamutMaskActivated(KoGamutMask*)));
    connect(this, SIGNAL(sigGamutMaskUnset()), m_resourceProvider, SLOT(slotGamutMaskUnset()));
    connect(this, SIGNAL(sigGamutMaskPreviewUpdate()), m_resourceProvider, SLOT(slotGamutMaskPreviewUpdate()));
    connect(KisPart::instance(), SIGNAL(sigDocumentRemoved(QString)), this, SLOT(slotDocumentRemoved(QString)));
}

void GamutMaskDock::slotGamutMaskEdit()
{
    if (!m_selectedMask) {
        return;
    }
    openMaskEditor();
}

void GamutMaskDock::openMaskEditor()
{
    m_dockerUI->maskPropertiesBox->setVisible(true);

    //shouldnt be nullptr here
    if (!m_selectedMask) {
        return;
    }

    m_dockerUI->maskTitleEdit->setText(m_selectedMask->title());
    m_dockerUI->maskDescriptionEdit->setPlainText(m_selectedMask->description());

    // open gamut mask template in the application
    QString maskTemplateFile = KoResourcePaths::findResource("data", "gamutmasks/GamutMaskTemplate.kra");
    dbgPlugins << "GamutMaskDock::slotGamutMaskEdit: maskTemplateFile"
               << maskTemplateFile;

    // open template document
    m_maskDocument = KisPart::instance()->createDocument();
    KisPart::instance()->addDocument(m_maskDocument);
    m_maskDocument->openUrl(QUrl::fromLocalFile(maskTemplateFile), KisDocument::DontAddToRecent);
    m_maskDocument->resetURL();

    KisShapeLayerSP shapeLayer = getShapeLayer();

    // pass only copies of shapes to the layer,
    // so the originals don't disappear from the mask later
    for (KoShape *shape: m_selectedMask->koShapes()) {
        shapeLayer->addShape(shape->cloneShape());
    }

    m_maskDocument->setPreActivatedNode(shapeLayer);

    // set document as active
    KisMainWindow* mainWindow = KisPart::instance()->currentMainwindow();
    Q_ASSERT(mainWindow);

    m_view = mainWindow->addViewAndNotifyLoadingCompleted(m_maskDocument);
    Q_ASSERT(m_view);

    for(KisView *view: KisPart::instance()->views()) {
        if (view->document() == m_maskDocument) {
            view->activateWindow();
            break;
        }
    }
}

void GamutMaskDock::cancelMaskEdit()
{
    if (m_selectedMask) {
        m_selectedMask->clearPreview();

        m_dockerUI->maskPropertiesBox->setVisible(false);

        if (m_resourceProvider->currentGamutMask() == m_selectedMask) {
            emit sigGamutMaskChanged(m_selectedMask);
        }
    }
}

void GamutMaskDock::saveSelectedMaskResource()
{
    if (m_selectedMask) {
        m_selectedMask->setDescription(m_dockerUI->maskDescriptionEdit->toPlainText());

        m_selectedMask->setMaskShapes(getShapesFromLayer());

        m_selectedMask->setImage(
                    m_maskDocument->image()->convertToQImage(m_maskDocument->image()->bounds()
                                                             , m_maskDocument->image()->profile()
                                                             )
                    );

        m_selectedMask->clearPreview();
        m_selectedMask->save();
    }
}

void GamutMaskDock::finalizeMaskSave()
{
    closeMaskDocument();
    m_dockerUI->maskPropertiesBox->setVisible(false);
}

KoGamutMask* GamutMaskDock::addDuplicateResource(QString newTitle)
{
    // construct a copy
    KoGamutMask* newMask = new KoGamutMask(m_selectedMask);

    QPair<QString,QFileInfo> maskFile = resolveMaskTitle(newTitle);
    QString maskTitle = maskFile.first;
    QFileInfo fileInfo = maskFile.second;

    newMask->setTitle(maskTitle);
    newMask->setFilename(fileInfo.filePath());

    newMask->setValid(true);

    KoResourceServer<KoGamutMask>* rServer = KoResourceServerProvider::instance()->gamutMaskServer();
    rServer->addResource(newMask, false);



    return newMask;
}

QPair<QString, QFileInfo> GamutMaskDock::resolveMaskTitle(QString suggestedTitle)
{
    KoResourceServer<KoGamutMask>* rServer = KoResourceServerProvider::instance()->gamutMaskServer();
    QString saveLocation = rServer->saveLocation();

    QString processedTitle = suggestedTitle.trimmed();
    QString newFilename = saveLocation + processedTitle.replace(QRegularExpression("\\s+"), "_") + ".kgm";

    QFileInfo fileInfo(newFilename);

    QStringList resourceBlacklist = rServer->blackListedFiles();

    int i = 1;
    int lastIndex = 0;
    while (fileInfo.exists()) {
        if (resourceBlacklist.contains(fileInfo.filePath())) {
            // allow to overwrite blacklisted masks
            break;
        } else {
            fileInfo.setFile(saveLocation + processedTitle + QString("%1").arg(i) + ".kgm");
            lastIndex = i;
            i++;
        }
    }

    QString maskTitle = (lastIndex > 0) ? (processedTitle + QString("%1").arg(lastIndex)) : processedTitle;

    return QPair<QString, QFileInfo>(maskTitle, fileInfo);
}

void GamutMaskDock::closeMaskDocument()
{
    if (m_maskDocument) {
        // HACK: set the document to not modified to bypass confirmation dialog
        // the close is already confirmed
        m_maskDocument->setModified(false);

        m_maskDocument->closeUrl();
        m_view->closeView();
        m_view->deleteLater();

        // HACK: set a flag that we are doing it ourselves, so the docker does not react to
        // removing signal from KisPart
        m_selfClosingMaskFile = true;
        KisPart::instance()->removeView(m_view);
        KisPart::instance()->removeDocument(m_maskDocument);
        m_selfClosingMaskFile = false;

        m_maskDocument = nullptr;
        m_view = nullptr;
    }
}

QList<KoShape*> GamutMaskDock::getShapesFromLayer()
{
    KisShapeLayerSP shapeLayer = getShapeLayer();

    // create a deep copy of the shapes to save in the mask,
    // otherwise they vanish when the template closes
    QList<KoShape*> newShapes;
    for (KoShape* sh: shapeLayer->shapes()) {
        KoShape* newShape = sh->cloneShape();
        newShapes.append(newShape);
    }

    return newShapes;
}

KisShapeLayerSP GamutMaskDock::getShapeLayer()
{
    KisNodeSP node = m_maskDocument->image()->rootLayer()->findChildByName("maskShapesLayer");
    return KisShapeLayerSP(dynamic_cast<KisShapeLayer*>(node.data()));
}

void GamutMaskDock::slotGamutMaskSave()
{
    QString newTitle = m_dockerUI->maskTitleEdit->text();
    // was this mask previously set to selectors?
    bool setToSelectors = (m_resourceProvider->currentGamutMask() == m_selectedMask);

    if (m_selectedMask->title() != newTitle) {
        // title has changed, rename
        if (!m_selectedMask) {
            return;
        }

        // construct a copy
        KoGamutMask* newMask = addDuplicateResource(newTitle);

        // delete old mask and select new
        slotGamutMaskDelete();
        m_selectedMask = newMask;
    }

    saveSelectedMaskResource();
    finalizeMaskSave();

    if (setToSelectors) {
        emit sigGamutMaskSet(m_selectedMask);
    }
}

void GamutMaskDock::slotGamutMaskCancelEdit()
{
    if (!m_selectedMask) {
        return;
    }

    bool cancelApproved = false;
    if (m_maskDocument->isModified()) {
        int res = QMessageBox::warning(this,
                                       i18nc("@title:window", "Krita"),
                                       i18n("<p>Gamut mask <b>'%1'</b> has been modified.</p><p>Discard changes?</p>"
                                            , m_selectedMask->title()),
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        switch (res) {
            case QMessageBox::Yes : {
                cancelApproved = true;
                break;
            }
            case QMessageBox::No : {
                cancelApproved = false;
                break;
            }
        }
    } else {
        cancelApproved = true;
    }

    if (cancelApproved) {
        cancelMaskEdit();
        closeMaskDocument();
    }
}

void GamutMaskDock::slotGamutMaskPreview()
{
    if (!m_selectedMask) {
        return;
    }

    m_selectedMask->setPreviewMaskShapes(getShapesFromLayer());
    emit sigGamutMaskPreviewUpdate();
}

void GamutMaskDock::slotGamutMaskSelected(KoGamutMask *mask)
{
    m_selectedMask = mask;

    if (m_selectedMask) {
        m_dockerUI->labelMaskName->setText(m_selectedMask->title());
    } else {
        m_dockerUI->labelMaskName->setText(i18n("Select a mask"));
    }
}

void GamutMaskDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);
}

void GamutMaskDock::unsetCanvas()
{
    setEnabled(false);
}


void GamutMaskDock::unsetResourceServer()
{
    KoResourceServer<KoGamutMask>* rServer = KoResourceServerProvider::instance()->gamutMaskServer();
    rServer->removeObserver(this);
}

void GamutMaskDock::removingResource(KoGamutMask *resource)
{
    // if deleting previously set mask, notify selectors to unset their mask
    if (resource == m_resourceProvider->currentGamutMask()) {
        emit sigGamutMaskUnset();
        m_selectedMask = nullptr;
        m_dockerUI->labelMaskName->setText(i18n("Select a mask"));
    }
}

void GamutMaskDock::resourceChanged(KoGamutMask *resource)
{
    // if currently set mask has been changed, notify selectors
    // TODO: is m_resourceProvider->currentGamutMask() == m_selectedMask?
    if (resource == m_resourceProvider->currentGamutMask()) {
        m_selectedMask = resource;
        emit sigGamutMaskChanged(resource);
    }
}

void GamutMaskDock::slotGamutMaskSaveNew()
{
    if (!m_selectedMask) {
        return;
    }

    KoGamutMask* newMask = addDuplicateResource(m_dockerUI->maskTitleEdit->text());
    m_selectedMask = newMask;

    saveSelectedMaskResource();
    finalizeMaskSave();
}

void GamutMaskDock::slotGamutMaskDelete()
{   
    if (!m_selectedMask) {
        return;
    }

    int res = QMessageBox::warning(this,
                                   i18nc("@title:window", "Krita"),
                                   i18n("Are you sure you want to delete mask <b>'%1'</b>?"
                                        , m_selectedMask->title()),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    // delete only if user confirms
    if (res == QMessageBox::Yes) {
        KoResourceServer<KoGamutMask>* rServer = KoResourceServerProvider::instance()->gamutMaskServer();
        // TODO: other resources do it this way, but the files stay on disk
        rServer->removeResourceAndBlacklist(m_selectedMask);

        m_selectedMask = nullptr;
        m_dockerUI->labelMaskName->setText(i18n("Select a mask"));
    }
}

void GamutMaskDock::slotGamutMaskSet()
{
    if (!m_selectedMask) {
        return;
    }

    emit sigGamutMaskSet(m_selectedMask);
}

void GamutMaskDock::slotDocumentRemoved(QString filename)
{
    if (!m_maskDocument) {
        return;
    }

    // HACK: we do not want to run this if it is we who close the file
    if (!m_selfClosingMaskFile) {
        // KisPart called, that a document will be removed
        // if it's ours, cancel the mask edit operation
        if (m_maskDocument->url().toLocalFile() == filename) {
            cancelMaskEdit();
        }
    }
}
