/*
 * SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "page_bundle_saver.h"
#include "ui_pagebundlesaver.h"
#include "dlg_create_bundle.h"

#include <kis_config.h>
#include <KoFileDialog.h>

PageBundleSaver::PageBundleSaver(KoResourceBundleSP bundle, QWidget *parent) :
    QWizardPage(parent),
    m_ui(new Ui::PageBundleSaver)
    , m_bundle(bundle)
{
    m_ui->setupUi(this);

    if (m_bundle) {
        QFileInfo fileInfo(m_bundle->filename());
        QString directory = fileInfo.path();
        m_ui->lblSaveLocation->setText(directory);

        m_bundleStorage = new KisBundleStorage(m_bundle->filename());
        QMap<QString, int> m_count;
        QStringList resourceTypes = QStringList() << ResourceType::Brushes << ResourceType::PaintOpPresets << ResourceType::Gradients << ResourceType::GamutMasks;
        #if defined HAVE_SEEXPR
        resourceTypes << ResourceType::SeExprScripts;
        #endif
        resourceTypes << ResourceType::Patterns << ResourceType::Palettes << ResourceType::Workspaces;

        m_resourceCount = "";
        QSet<QString> set;
        m_tags = "";

        for (const QString &type : resourceTypes) {
            QSharedPointer<KisResourceStorage::ResourceIterator> iter = m_bundleStorage->resources(type);
            int count = 0;
            while (iter->hasNext()) {
                iter->next();
                count++;
            }
            m_loaded_count[type] = count;
            if (count != 0) {
                m_count[type] = count;
                m_resourceCount += ResourceName::resourceTypeToName(type) + ": " + QString::number(count) + "<br>";
            }

            QSharedPointer<KisResourceStorage::TagIterator> tagIter = m_bundleStorage->tags(type);
            while (tagIter->hasNext()) {
                tagIter->next();
                set.insert(tagIter->tag()->name());
                m_loaded_tags.insert(tagIter->tag()->name());
            }
        }

        if (!m_resourceCount.isEmpty()) {
            m_ui->lblDetails->setText("<b>Resources</b><br>" + m_resourceCount);
        }

        if (!set.isEmpty()) {
            m_tags = "<b>Tags</b><br>" + set.toList().join(", ") + "<br>";
        }

        m_ui->lblDetails->setText(m_ui->lblDetails->text() + m_tags);


    } else {
        KisConfig cfg(true);
        m_ui->lblSaveLocation->setText(cfg.readEntry<QString>("BundleExportLocation", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
    }

    connect(m_ui->bnSelectSaveLocation, SIGNAL(clicked()), SLOT(selectSaveLocation()));

}

PageBundleSaver::~PageBundleSaver()
{
    delete m_ui;
}

QString PageBundleSaver::saveLocation() const
{
    return m_ui->lblSaveLocation->text();
}

void PageBundleSaver::selectSaveLocation()
{
    KoFileDialog dialog(this, KoFileDialog::OpenDirectory, "resourcebundlesavelocation");
    dialog.setDefaultDir(m_ui->lblSaveLocation->text());
    dialog.setCaption(i18n("Select a directory to save the bundle"));
    QString location = dialog.filename();
    m_ui->lblSaveLocation->setText(location);
}

void PageBundleSaver::showWarning()
{
     m_ui->lblSaveLocation->setStyleSheet(QString(" border: 1px solid red"));
}

void PageBundleSaver::onCountUpdated()
{
    DlgCreateBundle *wizard = qobject_cast<DlgCreateBundle*>(this->wizard());
    // QString bundleDetails;
    m_resourceCount = "";
    QMap<QString, int> map = wizard->m_count;
    bool resPresent = false;
    for (auto i = map.cbegin(), end = map.cend(); i != end; ++i) {
        if (i.value() != 0) {
            if (!resPresent) {
                resPresent = true;
                m_resourceCount = m_resourceCount + "<b>Resources</b>" + "<br>";
            }
            m_resourceCount = m_resourceCount + ResourceName::resourceTypeToName(i.key()) + ": " + QString::number(i.value()) + "<br>";
        }
    }
    m_ui->lblDetails->setText(m_resourceCount + m_tags);
}

void PageBundleSaver::onTagsUpdated()
{
    DlgCreateBundle *wizard = qobject_cast<DlgCreateBundle*>(this->wizard());
    m_tags = "";
    QSet<QString> set = wizard->m_tags;

    bool tagPresent = false;
    for (QSet<QString>::const_iterator it = set.constBegin(); it != set.constEnd(); ++it) {
        if (!tagPresent) {
            tagPresent = true;
            m_tags = m_tags + "<b>Tags</b>" + "<br>";
        }
        if (it + 1 == set.constEnd()) {
            m_tags = m_tags + *it;
        } else {
            m_tags = m_tags + *it + ", ";
        }
    }

    m_tags = m_tags + "<br>";
    m_ui->lblDetails->setText(m_resourceCount + m_tags);
}

void PageBundleSaver::removeWarning()
{
     m_ui->lblSaveLocation->setStyleSheet(QString(""));
}
