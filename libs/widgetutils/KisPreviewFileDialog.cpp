/*
 *  Copyright (c) 2020 Halla Rempt <halla@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisPreviewFileDialog.h"

#include <QSplitter>
#include <QLabel>
#include <QFileIconProvider>
#include <QDebug>

#include <klocalizedstring.h>

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
    m_iconProvider = new KisFileIconProvider(devicePixelRatioF());
    setIconProvider(m_iconProvider);

    m_preview = new QLabel(i18n("Preview"), this);
    m_preview->setAlignment(Qt::AlignCenter);
    m_preview->setMinimumWidth(256);

    connect(this, SIGNAL(currentChanged(const QString&)), this, SLOT(onCurrentChanged(const QString&)));
}

void KisPreviewFileDialog::resetIconProvider()
{
    QSplitter *splitter = findChild<QSplitter*>();
    if (splitter) {
        splitter->addWidget(m_preview);
        resize(width() + m_preview->width(), height());
    }

    setIconProvider(m_iconProvider);
}

void KisPreviewFileDialog::onCurrentChanged(const QString &path)
{
    if (m_preview) {
        QIcon icon;
        if (s_iconCreator && s_iconCreator->createFileIcon(path, icon, devicePixelRatioF(), QSize(512, 512))) {
            m_preview->setPixmap(icon.pixmap(m_preview->width(), m_preview->height()));
        }
        else {
            m_preview->setText(i18n("No Preview"));
        }
    }
}

KisAbstractFileIconCreator::KisAbstractFileIconCreator()
{

}

KisAbstractFileIconCreator::~KisAbstractFileIconCreator()
{

}
