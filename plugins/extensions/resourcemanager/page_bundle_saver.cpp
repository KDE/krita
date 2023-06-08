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

    if (bundle) {
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
    // QString bundleDetails;
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
