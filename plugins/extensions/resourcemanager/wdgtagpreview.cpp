/*
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "wdgtagpreview.h"
#include "ui_wdgtagpreview.h"
#include "KisTagLabel.h"

#include "KisTagSelectionWidget.h"
#include "KoID.h"
#include <wdgtagselection.h>
#include <KisTag.h>
#include <KisTagModelProvider.h>
#include <KisWrappableHBoxLayout.h>
#include <KisTagSelectionWidget.h>
#include "KisBundleStorage.h"


WdgTagPreview::WdgTagPreview(QString resourceType, KoResourceBundleSP bundle, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::WdgTagPreview),
    m_resourceType(resourceType),
    m_tagsController(0),
    m_bundle(bundle)
{
    m_ui->setupUi(this);

    m_resourceType = (resourceType == "presets" ? ResourceType::PaintOpPresets : resourceType);

    m_wdgResourcesTags = new KisTagSelectionWidget(this, false);
    m_ui->verticalLayout_2->addWidget(m_wdgResourcesTags);

    m_tagsController = new KisWdgTagSelectionControllerBundleTags(m_wdgResourcesTags, true);
    m_tagsController->setResourceType(m_resourceType);

    connect(m_tagsController, SIGNAL(tagAdded(KoID)), this, SLOT(onTagAdded(KoID)));
    connect(m_tagsController, SIGNAL(tagRemoved(KoID)), this, SLOT(onTagRemoved(KoID)));

    m_layout = new KisWrappableHBoxLayout();

    m_ui->widget->setLayout(m_layout);

    KisTagModel* model = new KisTagModel(m_resourceType);
    for (int i = 0; i < model->rowCount(); ++i) {
        QModelIndex idx = model->index(i, 0);
        QString name  = model->data(idx, Qt::UserRole + KisAbstractResourceModel::Name).toString();
        if (name == "All" || name == "All untagged") continue;
        KisTagLabel *label = new KisTagLabel(name);
        m_layout->addWidget(label);
    }

    if (m_bundle) {
        KisBundleStorage *bundleStorage = new KisBundleStorage(m_bundle->filename());

        QSharedPointer<KisResourceStorage::TagIterator> tagIter = bundleStorage->tags(m_resourceType);
        while (tagIter->hasNext()) {
            tagIter->next();
            KoID custom = KoID(tagIter->tag()->url(), tagIter->tag()->name());
            m_tagsController->addTag(custom);
        }
    }
}

WdgTagPreview::~WdgTagPreview()
{
    delete m_ui;
}

void WdgTagPreview::onTagAdded(KoID custom)
{
    KisTagModel* model = new KisTagModel(m_resourceType);
    KisTagSP tagSP = model->tagForUrl(custom.id());

    for (int i = m_layout->count() - 1; i >= 0; --i) {
        KisTagLabel *label = qobject_cast<KisTagLabel*>(m_layout->itemAt(i)->widget());
        if (label && label->getText() == tagSP->name()) {
            m_layout->removeWidget(label);
            delete label;
        }
    }

    Q_EMIT tagsAdded(tagSP);
}
void WdgTagPreview::onTagRemoved(KoID custom)
{
    KisTagModel* model = new KisTagModel(m_resourceType);
    KisTagSP tagSP = model->tagForUrl(custom.id());

    KisTagLabel *label = new KisTagLabel(tagSP->name());
    m_layout->addWidget(label);

    Q_EMIT tagsRemoved(tagSP);
}
