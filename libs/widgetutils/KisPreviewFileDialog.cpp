/*
 *  SPDX-FileCopyrightText: 2020 Halla Rempt <halla@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisPreviewFileDialog.h"

#include <QSplitter>
#include <QLabel>
#include <QFileIconProvider>
#include <QDebug>
#include <QToolButton>
#include <QHBoxLayout>

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>
#include <kis_icon_utils.h>

KisAbstractFileIconCreator *KisPreviewFileDialog::s_iconCreator {0};

KisFileIconProvider::KisFileIconProvider(qreal devicePixelRatioF)
    : QFileIconProvider()
    , m_devicePixelRatioF(devicePixelRatioF)
{
}

QIcon KisFileIconProvider::icon(const QFileIconProvider::IconType type) const
{
    return QFileIconProvider::icon(type);
}

QIcon KisFileIconProvider::icon(const QFileInfo &fi) const
{
    QIcon icon;
    if (KisPreviewFileDialog::s_iconCreator && KisPreviewFileDialog::s_iconCreator->createFileIcon(fi.filePath(), icon, m_devicePixelRatioF, QSize(512, 512))) {
        return icon;
    }
    return QFileIconProvider::icon(fi.path());
}

KisPreviewFileDialog::KisPreviewFileDialog(QWidget *parent, const QString &caption, const QString &directory, const QString &filter)
    : QFileDialog(parent, caption, directory, filter)
{
    KConfigGroup group = KSharedConfig::openConfig()->group("KisPreviewFileDialog");
    if (group.readEntry("show_thumbnails", false)) {
        m_iconProvider = new KisFileIconProvider(devicePixelRatioF());
    }

    m_preview = new QLabel(i18n("Preview"), this);
    m_preview->setAlignment(Qt::AlignCenter);
    m_preview->setMinimumWidth(256);

    m_previewToggle = new QToolButton(this);
    m_previewToggle->setCheckable(true);
    m_previewToggle->setChecked(group.readEntry("show_preview", true));
    m_previewToggle->setIcon(KisIconUtils::loadIcon("preview"));
    m_previewToggle->setToolTip(i18n("Toggle Preview"));
    connect(m_previewToggle, SIGNAL(toggled(bool)), SLOT(previewToggled(bool)));

    connect(this, SIGNAL(currentChanged(const QString&)), this, SLOT(onCurrentChanged(const QString&)));
}

void KisPreviewFileDialog::resetIconProvider()
{
    QSplitter *splitter = findChild<QSplitter*>();
    if (splitter) {
        splitter->addWidget(m_preview);
        resize(width() + m_preview->width(), height());
    }

    QHBoxLayout *layout = findChild<QHBoxLayout*>();
    if (layout) {
        layout->addWidget(m_previewToggle);
    }

    KConfigGroup group = KSharedConfig::openConfig()->group("File Dialogs");
    if (group.readEntry("show_thumbnails", false)) {
        setIconProvider(m_iconProvider);
    }
}

void KisPreviewFileDialog::onCurrentChanged(const QString &path)
{
    if (m_preview) {
        QIcon icon;
        if (s_iconCreator && s_iconCreator->createFileIcon(path, icon, devicePixelRatioF(), QSize(512, 512), true)) {
            m_preview->setPixmap(icon.pixmap(m_preview->width(), m_preview->height()));
        }
        else {
            m_preview->setText(i18n("No Preview"));
        }
    }
}

void KisPreviewFileDialog::previewToggled(bool showPreview)
{
    KConfigGroup group = KSharedConfig::openConfig()->group("KisPreviewFileDialog");
    group.writeEntry("show_preview", showPreview);
    m_preview->setVisible(showPreview);
}


KisAbstractFileIconCreator::KisAbstractFileIconCreator()
{
}

KisAbstractFileIconCreator::~KisAbstractFileIconCreator()
{
}
