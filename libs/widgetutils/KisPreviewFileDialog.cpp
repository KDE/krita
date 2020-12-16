/*
 *  Copyright (c) 2020 Halla Rempt <halla@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisPreviewFileDialog.h"

#include <QGridLayout>
#include <QVBoxLayout>
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

    QGridLayout *layout = dynamic_cast<QGridLayout*>(this->layout());
    if (layout) {

        QVBoxLayout *box = new QVBoxLayout();
        m_preview = new QLabel(i18n("Preview"), this);
        m_preview->setAlignment(Qt::AlignCenter);
        m_preview->setFixedSize(512, 512);
        box->addWidget(m_preview);
        box->addStretch();

        QList< QPair<QLayoutItem*, QList<int> > > movedItems;
        for(int i = 0; i < layout->count(); i++)
        {
            int row, column, rowSpan, columnSpan;
            layout->getItemPosition(i,&row,&column,&rowSpan,&columnSpan);
            if (row > 2)
            {
                QList<int> list;
                list << row << column << rowSpan << columnSpan;
                movedItems << qMakePair(layout->takeAt(i),list);
                i--;
            }
        }
        for(int i = 0; i < movedItems.count(); i++)
        {
            layout->addItem(movedItems[i].first,
                            movedItems[i].second[0],
                    movedItems[i].second[1],
                    movedItems[i].second[2],
                    movedItems[i].second[3]
                    );
        }

        layout->addItem(box,1,3,1,1);
    }
    connect(this, SIGNAL(currentChanged(const QString&)), this, SLOT(onCurrentChanged(const QString&)));

}

void KisPreviewFileDialog::resetIconProvider()
{
    setIconProvider(m_iconProvider);
}

void KisPreviewFileDialog::onCurrentChanged(const QString &path)
{
    if (m_preview) {
        QIcon icon;
        if (s_iconCreator && s_iconCreator->createFileIcon(path, icon, devicePixelRatioF(), QSize(512, 512))) {
            m_preview->setPixmap(icon.pixmap(512, 512));
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
