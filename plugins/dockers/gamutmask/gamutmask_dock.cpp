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
#include <KoCanvasResourceProvider.h>
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
#include <KoColorBackground.h>
#include <KoShapeStroke.h>

#include <ctime>

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
    , m_selfClosingTemplate(false)
    , m_externalTemplateClose(false)
    , m_creatingNewMask(false)
    , m_templatePrevSaved(false)
    , m_selfSelectingMask(false)
    , m_selectedMask(nullptr)
    , m_maskDocument(nullptr)
    , m_view(nullptr)
{
    m_dockerUI    = new GamutMaskChooserUI();

    m_dockerUI->bnMaskEditor->setIcon(KisIconUtils::loadIcon("dirty-preset"));
    m_dockerUI->bnMaskDelete->setIcon(KisIconUtils::loadIcon("deletelayer"));
    m_dockerUI->bnMaskNew->setIcon(KisIconUtils::loadIcon("list-add"));
    m_dockerUI->bnMaskDuplicate->setIcon(KisIconUtils::loadIcon("duplicatelayer"));

    m_dockerUI->maskPropertiesBox->setVisible(false);
    m_dockerUI->bnSaveMask->setIcon(KisIconUtils::loadIcon("document-save"));
    m_dockerUI->bnCancelMaskEdit->setIcon(KisIconUtils::loadIcon("dialog-cancel"));
    m_dockerUI->bnPreviewMask->setIcon(KisIconUtils::loadIcon("visible"));

    QRegularExpression maskTitleRegex("^[-_\\(\\)\\sA-Za-z0-9]+$");
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
    connect(m_dockerUI->bnMaskNew          , SIGNAL(clicked())               , SLOT(slotGamutMaskCreateNew()));
    connect(m_dockerUI->bnMaskDelete          , SIGNAL(clicked())               , SLOT(slotGamutMaskDelete()));
    connect(m_dockerUI->bnMaskDuplicate       , SIGNAL(clicked())               , SLOT(slotGamutMaskDuplicate()));

    setWidget(m_dockerUI);
}

GamutMaskDock::~GamutMaskDock()
{
    KoResourceServer<KoGamutMask>* rServer = KoResourceServerProvider::instance()->gamutMaskServer();
    rServer->removeObserver(this);
}

void GamutMaskDock::setViewManager(KisViewManager* kisview)
{
    m_resourceProvider = kisview->canvasResourceProvider();

    selectMask(m_resourceProvider->currentGamutMask());

    connect(this, SIGNAL(sigGamutMaskSet(KoGamutMask*)), m_resourceProvider, SLOT(slotGamutMaskActivated(KoGamutMask*)), Qt::UniqueConnection);
    connect(this, SIGNAL(sigGamutMaskChanged(KoGamutMask*)), m_resourceProvider, SLOT(slotGamutMaskActivated(KoGamutMask*)), Qt::UniqueConnection);
    connect(this, SIGNAL(sigGamutMaskUnset()), m_resourceProvider, SLOT(slotGamutMaskUnset()), Qt::UniqueConnection);
    connect(this, SIGNAL(sigGamutMaskPreviewUpdate()), m_resourceProvider, SLOT(slotGamutMaskPreviewUpdate()), Qt::UniqueConnection);
    connect(KisPart::instance(), SIGNAL(sigDocumentRemoved(QString)), this, SLOT(slotDocumentRemoved(QString)), Qt::UniqueConnection);
}

void GamutMaskDock::slotGamutMaskEdit()
{
    if (!m_selectedMask) {
        return;
    }
    openMaskEditor();
}

bool GamutMaskDock::openMaskEditor()
{
    if (!m_selectedMask) {
        return false;
    }

    // find the template resource first, so we can abort the action early on
    QString maskTemplateFile = KoResourcePaths::findResource("ko_gamutmasks", "GamutMaskTemplate.kra");
    if (maskTemplateFile.isEmpty() || maskTemplateFile.isNull() || !QFile::exists(maskTemplateFile)) {
        dbgPlugins << "GamutMaskDock::openMaskEditor(): maskTemplateFile (" << maskTemplateFile << ") was not found on the system";
        getUserFeedback(i18n("Could not open gamut mask for editing."),
                        i18n("The editor template was not found."),
                        QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Critical);
        return false;
    }

    m_dockerUI->maskPropertiesBox->setVisible(true);
    m_dockerUI->maskPropertiesBox->setEnabled(true);
    m_dockerUI->editControlsBox->setEnabled(false);
    m_dockerUI->editControlsBox->setVisible(false);

    m_dockerUI->maskTitleEdit->setText(m_selectedMask->title());
    m_dockerUI->maskDescriptionEdit->setPlainText(m_selectedMask->description());

    m_maskDocument = KisPart::instance()->createDocument();
    KisPart::instance()->addDocument(m_maskDocument);
    m_maskDocument->openUrl(QUrl::fromLocalFile(maskTemplateFile), KisDocument::DontAddToRecent);

    // template document needs a proper autogenerated filename,
    // to avoid collision with other documents,
    // otherwise bugs happen when slotDocumentRemoved is called
    // (e.g. user closes another view, the template stays open, but the edit operation is canceled)
    m_maskDocument->setInfiniteAutoSaveInterval();
    QString maskPath = QString("%1%2%3_%4.kra")
            .arg(QDir::tempPath())
            .arg(QDir::separator())
            .arg("GamutMaskTemplate")
            .arg(std::time(nullptr));
    m_maskDocument->setUrl(QUrl::fromLocalFile(maskPath));
    m_maskDocument->setLocalFilePath(maskPath);

    KisShapeLayerSP shapeLayer = getShapeLayer();

    // pass only copies of shapes to the layer,
    // so the originals don't disappear from the mask later
    for (KoShape *shape: m_selectedMask->koShapes()) {
        KoShape* newShape = shape->cloneShape();
        newShape->setStroke(KoShapeStrokeModelSP());
        newShape->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(QColor(255,255,255))));
        shapeLayer->addShape(newShape);
    }

    m_maskDocument->setPreActivatedNode(shapeLayer);

    // set document as active
    KisMainWindow* mainWindow = KisPart::instance()->currentMainwindow();
    KIS_ASSERT(mainWindow);

    m_view = mainWindow->addViewAndNotifyLoadingCompleted(m_maskDocument);
    KIS_ASSERT(m_view);

    for(KisView *view: KisPart::instance()->views()) {
        if (view->document() == m_maskDocument) {
            view->activateWindow();
            break;
        }
    }

    connect(m_view->viewManager(), SIGNAL(viewChanged()), this, SLOT(slotViewChanged()));
    connect(m_maskDocument, SIGNAL(completed()), this, SLOT(slotDocumentSaved()));

    return true;
}

void GamutMaskDock::cancelMaskEdit()
{
    if (m_creatingNewMask) {
        deleteMask();
    }

    if (m_selectedMask) {
        m_selectedMask->clearPreview();

        if (m_resourceProvider->currentGamutMask() == m_selectedMask) {
            emit sigGamutMaskChanged(m_selectedMask);
        }
    }

    closeMaskDocument();
}

void GamutMaskDock::selectMask(KoGamutMask *mask, bool notifyItemChooser)
{
    if (!mask) {
        return;
    }

    m_selectedMask = mask;

    if (notifyItemChooser) {
        m_selfSelectingMask = true;
        m_dockerUI->maskChooser->setCurrentResource(m_selectedMask);
        m_selfSelectingMask = false;
    }

    emit sigGamutMaskSet(m_selectedMask);
}

bool GamutMaskDock::saveSelectedMaskResource()
{
    if (!m_selectedMask || !m_maskDocument) {
        return false;
    }

    bool maskSaved = false;

    if (m_selectedMask) {
        QList<KoShape*> shapes = getShapesFromLayer();

        if (shapes.count() > 0) {
            m_selectedMask->setMaskShapes(shapes);

            m_selectedMask->setImage(
                        m_maskDocument->image()->convertToQImage(m_maskDocument->image()->bounds()
                                                                 , m_maskDocument->image()->profile()
                                                                 )
                        );

            m_selectedMask->setDescription(m_dockerUI->maskDescriptionEdit->toPlainText());

            m_selectedMask->clearPreview();
            m_selectedMask->save();
            maskSaved = true;
        } else {
            getUserFeedback(i18n("Saving of gamut mask '%1' was aborted.", m_selectedMask->title()),
                            i18n("<p>The mask template is invalid.</p>"
                                 "<p>Please check that:"
                                 "<ul>"
                                 "<li>your template contains a vector layer named 'maskShapesLayer'</li>"
                                 "<li>there are one or more vector shapes on the 'maskShapesLayer'</li>"
                                 "</ul></p>"
                                 ),
                            QMessageBox::Ok, QMessageBox::Ok);
        }
    }

    return maskSaved;
}

void GamutMaskDock::deleteMask()
{
    KoResourceServer<KoGamutMask>* rServer = KoResourceServerProvider::instance()->gamutMaskServer();
    rServer->removeResourceAndBlacklist(m_selectedMask);
    m_selectedMask = nullptr;
}

int GamutMaskDock::getUserFeedback(QString text, QString informativeText,
                                   QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton,
                                   QMessageBox::Icon severity)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(i18nc("@title:window", "Krita"));
    msgBox.setText(QString("<p><b>%1</b></p>").arg(text));
    msgBox.setInformativeText(informativeText);
    msgBox.setStandardButtons(buttons);
    msgBox.setDefaultButton(defaultButton);
    msgBox.setIcon(severity);
    int res = msgBox.exec();

    return res;
}

int GamutMaskDock::saveOrCancel(QMessageBox::StandardButton defaultAction)
{
    int response = 0;

    if (m_maskDocument->isModified()) {
        response = getUserFeedback(i18n("Gamut mask <b>'%1'</b> has been modified.", m_selectedMask->title()),
                                   i18n("Do you want to save it?"),
                                   QMessageBox::Cancel | QMessageBox::Close | QMessageBox::Save, defaultAction);

    } else if (m_templatePrevSaved && defaultAction != QMessageBox::Close) {
        response = QMessageBox::Save;

    } else if (!m_templatePrevSaved) {
        response = QMessageBox::Close;

    } else {
        response = defaultAction;
    }

    switch (response) {
        case QMessageBox::Save : {
            slotGamutMaskSave();
            break;
    }
        case QMessageBox::Close : {
            cancelMaskEdit();
            break;
        }
    }

    return response;
}

KoGamutMask *GamutMaskDock::createMaskResource(KoGamutMask* sourceMask, QString newTitle)
{
    m_creatingNewMask = true;

    KoGamutMask* newMask = nullptr;
    if (sourceMask) {
        newMask = new KoGamutMask(sourceMask);
        newMask->setImage(sourceMask->image());
    } else {
        newMask = new KoGamutMask();

        QString defaultPreviewPath = KoResourcePaths::findResource("ko_gamutmasks", "empty_mask_preview.png");
        KIS_SAFE_ASSERT_RECOVER_NOOP(!(defaultPreviewPath.isEmpty() || defaultPreviewPath.isNull() || !QFile::exists(defaultPreviewPath)));

        newMask->setImage(QImage(defaultPreviewPath, "PNG"));
    }

    QPair<QString,QFileInfo> maskFile = resolveMaskTitle(newTitle);
    QString maskTitle = maskFile.first;
    QFileInfo fileInfo = maskFile.second;

    newMask->setTitle(maskTitle);
    newMask->setFilename(fileInfo.filePath());

    newMask->setValid(true);

    KoResourceServer<KoGamutMask>* rServer = KoResourceServerProvider::instance()->gamutMaskServer();
    rServer->removeFromBlacklist(newMask);
    rServer->addResource(newMask, false);

    return newMask;
}

QPair<QString, QFileInfo> GamutMaskDock::resolveMaskTitle(QString suggestedTitle)
{
    KoResourceServer<KoGamutMask>* rServer = KoResourceServerProvider::instance()->gamutMaskServer();
    QString saveLocation = rServer->saveLocation();
    QString processedTitle = suggestedTitle.trimmed();

    QString resourceName = processedTitle;
    while (rServer->resourceByName(resourceName)) {
        resourceName = resourceName + QString(" (Copy)");
    }

    QString maskTitle = resourceName;
    QString maskFile = maskTitle + ".kgm";
    QString path = saveLocation + maskFile.replace(QRegularExpression("\\s+"), "_");
    QFileInfo fileInfo(path);

    return QPair<QString, QFileInfo>(maskTitle, fileInfo);
}

void GamutMaskDock::closeMaskDocument()
{
    if (!m_externalTemplateClose) {
        if (m_maskDocument) {
            // set the document to not modified to bypass confirmation dialog
            // the close is already confirmed
            m_maskDocument->setModified(false);

            m_maskDocument->closeUrl();
            m_view->closeView();
            m_view->deleteLater();

            // set a flag that we are doing it ourselves, so the docker does not react to
            // removing signal from KisPart
            m_selfClosingTemplate = true;
            KisPart::instance()->removeView(m_view);
            KisPart::instance()->removeDocument(m_maskDocument);
            m_selfClosingTemplate = false;
        }
    }

    m_dockerUI->maskPropertiesBox->setVisible(false);
    m_dockerUI->editControlsBox->setVisible(true);
    m_dockerUI->editControlsBox->setEnabled(true);

    disconnect(m_view->viewManager(), SIGNAL(viewChanged()), this, SLOT(slotViewChanged()));
    disconnect(m_maskDocument, SIGNAL(completed()), this, SLOT(slotDocumentSaved()));

    // the template file is meant as temporary, if the user saved it, delete now
    if (QFile::exists(m_maskDocument->localFilePath())) {
        QFile::remove(m_maskDocument->localFilePath());
    }

    m_maskDocument = nullptr;
    m_view = nullptr;
    m_creatingNewMask = false;
    m_templatePrevSaved = false;
}

QList<KoShape*> GamutMaskDock::getShapesFromLayer()
{
    KisShapeLayerSP shapeLayer = getShapeLayer();

    // create a deep copy of the shapes to save in the mask,
    // otherwise they vanish when the template closes
    QList<KoShape*> newShapes;

    if (shapeLayer) {
        for (KoShape* sh: shapeLayer->shapes()) {
            KoShape* newShape = sh->cloneShape();
            KoShapeStrokeSP border(new KoShapeStroke(0.5f, Qt::white));
            newShape->setStroke(border);
            newShape->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(QColor(255,255,255,0))));
            newShapes.append(newShape);
        }
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
    if (!m_selectedMask || !m_maskDocument) {
        return;
    }

    QString newTitle = m_dockerUI->maskTitleEdit->text();

    if (m_selectedMask->title() != newTitle) {
        // title has changed, rename
        KoGamutMask* newMask = createMaskResource(m_selectedMask, newTitle);

        // delete old mask and select new
        deleteMask();
        selectMask(newMask);
    }

    bool maskSaved = saveSelectedMaskResource();
    if (maskSaved) {
        emit sigGamutMaskSet(m_selectedMask);
        closeMaskDocument();
    }
}

void GamutMaskDock::slotGamutMaskCancelEdit()
{
    if (!m_selectedMask) {
        return;
    }

    saveOrCancel(QMessageBox::Close);
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
    if (!m_selfSelectingMask) {
        if (m_maskDocument) {
            int res = saveOrCancel();
            if (res == QMessageBox::Cancel) {
                return;
            }
        }

        selectMask(mask, false);
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
    }
}

void GamutMaskDock::resourceChanged(KoGamutMask *resource)
{
    // if currently set mask has been changed, notify selectors
    if (resource == m_resourceProvider->currentGamutMask()) {
        selectMask(resource);
    }
}

void GamutMaskDock::slotGamutMaskCreateNew()
{
    KoGamutMask* newMask = createMaskResource(nullptr, "new mask");
    selectMask(newMask);

    bool editorOpened = openMaskEditor();
    if (!editorOpened) {
        deleteMask();
    }
}

void GamutMaskDock::slotGamutMaskDuplicate()
{
    if (!m_selectedMask) {
        return;
    }

    KoGamutMask* newMask = createMaskResource(m_selectedMask, m_selectedMask->title());
    selectMask(newMask);

    bool editorOpened = openMaskEditor();
    if (!editorOpened) {
        deleteMask();
    }
}

void GamutMaskDock::slotGamutMaskDelete()
{   
    if (!m_selectedMask) {
        return;
    }

    int res = getUserFeedback(i18n("Are you sure you want to delete mask <b>'%1'</b>?"
                                   , m_selectedMask->title()));

    if (res == QMessageBox::Yes) {
        deleteMask();
    }
}

void GamutMaskDock::slotDocumentRemoved(QString filename)
{
    if (!m_maskDocument) {
        return;
    }

    m_externalTemplateClose = true;

    // we do not want to run this if it is we who close the file
    if (!m_selfClosingTemplate) {
        // KisPart called, that a document will be removed
        // if it's ours, cancel the mask edit operation
        if (m_maskDocument->url().toLocalFile() == filename) {
            m_maskDocument->waitForSavingToComplete();
            saveOrCancel();
        }
    }

    m_externalTemplateClose = false;
}

void GamutMaskDock::slotViewChanged()
{
    if (!m_maskDocument || !m_view) {
        return;
    }

    if (m_view->viewManager()->document() == m_maskDocument) {
        m_dockerUI->maskPropertiesBox->setEnabled(true);
    } else {
        m_dockerUI->maskPropertiesBox->setEnabled(false);
    }
}

void GamutMaskDock::slotDocumentSaved()
{
    m_templatePrevSaved = true;
}
