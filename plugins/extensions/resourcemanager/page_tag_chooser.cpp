#include "page_tag_chooser.h"
#include "ui_pagetagchooser.h"
#include "dlg_create_bundle.h"

#include <QProcessEnvironment>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>
#include <QGridLayout>
#include <QTableWidget>
#include <QPainter>
#include <QListWidget>

#include <KisImportExportManager.h>
#include <KoDocumentInfo.h>
#include <KoFileDialog.h>
#include <kis_icon.h>
#include <KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <kstandardguiitem.h>
#include <KisTagModel.h>
#include "wdgtagpreview.h"
#include <KisTag.h>


#include <kis_workspace_resource.h>
#include <brushengine/kis_paintop_preset.h>
#include <dlg_embed_tags.h>

#include <kis_config.h>

#define ICON_SIZE 48

PageTagChooser::PageTagChooser(KoResourceBundleSP bundle, QWidget *parent) :
    QWizardPage(parent),
    m_ui(new Ui::PageTagChooser)
    , m_bundle(bundle)
{
    m_ui->setupUi(this);

    KoDocumentInfo info;
    info.updateParameters();

    WdgTagPreview* tabBrushes = new WdgTagPreview(ResourceType::Brushes);
    WdgTagPreview* tabBrushPresets = new WdgTagPreview(ResourceType::PaintOpPresets);
    WdgTagPreview* tabGradients = new WdgTagPreview(ResourceType::Gradients);
    WdgTagPreview* tabGamutMasks = new WdgTagPreview(ResourceType::GamutMasks);
    WdgTagPreview* tabPatterns = new WdgTagPreview(ResourceType::Patterns);
    WdgTagPreview* tabPalettes = new WdgTagPreview(ResourceType::Palettes);
    WdgTagPreview* tabWorkspaces = new WdgTagPreview(ResourceType::Workspaces);

    m_ui->tabWidget->addTab(tabBrushes, i18n("Brushes"));
    m_ui->tabWidget->addTab(tabBrushPresets, i18n("Brush Presets"));
    m_ui->tabWidget->addTab(tabGradients, i18n("Gradients"));
    m_ui->tabWidget->addTab(tabGamutMasks, i18n("Gamut Masks"));
    m_ui->tabWidget->addTab(tabPatterns, i18n("Patterns"));
    m_ui->tabWidget->addTab(tabPalettes, i18n("Palettes"));
    m_ui->tabWidget->addTab(tabWorkspaces, i18n("Workspaces"));

    connect(tabBrushes, SIGNAL(tagsAdded(KisTagSP)), this, SLOT(addSelected(KisTagSP)));
    connect(tabBrushPresets, SIGNAL(tagsAdded(KisTagSP)), this, SLOT(addSelected(KisTagSP)));
    connect(tabGradients, SIGNAL(tagsAdded(KisTagSP)), this, SLOT(addSelected(KisTagSP)));
    connect(tabGamutMasks, SIGNAL(tagsAdded(KisTagSP)), this, SLOT(addSelected(KisTagSP)));
    connect(tabPatterns, SIGNAL(tagsAdded(KisTagSP)), this, SLOT(addSelected(KisTagSP)));
    connect(tabPalettes, SIGNAL(tagsAdded(KisTagSP)), this, SLOT(addSelected(KisTagSP)));
    connect(tabWorkspaces, SIGNAL(tagsAdded(KisTagSP)), this, SLOT(addSelected(KisTagSP)));

    connect(tabBrushes, SIGNAL(tagsRemoved(KisTagSP)), this, SLOT(removeSelected(KisTagSP)));
    connect(tabBrushPresets, SIGNAL(tagsRemoved(KisTagSP)), this, SLOT(removeSelected(KisTagSP)));
    connect(tabGradients, SIGNAL(tagsRemoved(KisTagSP)), this, SLOT(removeSelected(KisTagSP)));
    connect(tabGamutMasks, SIGNAL(tagsRemoved(KisTagSP)), this, SLOT(removeSelected(KisTagSP)));
    connect(tabPatterns, SIGNAL(tagsRemoved(KisTagSP)), this, SLOT(removeSelected(KisTagSP)));
    connect(tabPalettes, SIGNAL(tagsRemoved(KisTagSP)), this, SLOT(removeSelected(KisTagSP)));
    connect(tabWorkspaces, SIGNAL(tagsRemoved(KisTagSP)), this, SLOT(removeSelected(KisTagSP)));
}

PageTagChooser::~PageTagChooser()
{
    delete m_ui;
}

QList<int> PageTagChooser::selectedTagIds()
{
    return m_selectedTagIds;
}

void PageTagChooser::addSelected(KisTagSP tagSP)
{
    m_selectedTagIds.append(tagSP->id());
    updateTags(true, tagSP->name());
}

void PageTagChooser::removeSelected(KisTagSP tagSP)
{
    m_selectedTagIds.removeAll(tagSP->id());
    updateTags(false, tagSP->name());
}

void PageTagChooser::updateTags(bool flag, QString tag)
{
    DlgCreateBundle *wizard = qobject_cast<DlgCreateBundle*>(this->wizard());
    if (wizard) {
        if (flag) {
            wizard->m_tags.insert(tag);
        } else {
            wizard->m_tags.remove(tag);
        }
    }

    emit tagsUpdated();
}
