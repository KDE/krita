/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2005 Peter Simonsson <psn@linux.se>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KisDetailsPane.h"

#include <QStandardItemModel>
#include <QKeyEvent>

////////////////////////////////////
// class KisDetailsPane
///////////////////////////////////

struct KisDetailsPanePrivate
{
    QStandardItemModel m_model;
};

KisDetailsPane::KisDetailsPane(QWidget* parent, const QString& header)
        : QWidget(parent),
        Ui_KisDetailsPaneBase(),
        d(new KisDetailsPanePrivate())
{
    d->m_model.setHorizontalHeaderItem(0, new QStandardItem(header));

    setupUi(this);

    m_documentList->installEventFilter(this);
    m_documentList->setIconSize(QSize(IconExtent, IconExtent));
    m_documentList->setModel(&d->m_model);
    m_splitter->setSizes(QList<int>() << 2 << 1);

    changePalette();

    connect(m_documentList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(selectionChanged(QModelIndex)));
    connect(m_documentList, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(openFile(QModelIndex)));
    connect(m_openButton, SIGNAL(clicked()), this, SLOT(openFile()));
}

KisDetailsPane::~KisDetailsPane()
{
    delete d;
}

bool KisDetailsPane::eventFilter(QObject* watched, QEvent* e)
{
    if (watched == m_documentList) {
        if ((e->type() == QEvent::Resize) && isVisible()) {
            emit splitterResized(this, m_splitter->sizes());
        }

        if ((e->type() == QEvent::KeyPress)) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);

            if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
                openFile();
            }
        }
    }

    return false;
}

void KisDetailsPane::resizeSplitter(KisDetailsPane* sender, const QList<int>& sizes)
{
    if (sender == this)
        return;

    m_splitter->setSizes(sizes);
}

void KisDetailsPane::openFile()
{
    QModelIndex index = m_documentList->selectionModel()->currentIndex();
    openFile(index);
}

void KisDetailsPane::changePalette()
{
    QPalette p = palette();
    p.setBrush(QPalette::Base, QColor(Qt::transparent));
    p.setColor(QPalette::Text, p.color(QPalette::Normal, QPalette::Foreground));
    m_detailsLabel->setPalette(p);
}

QStandardItemModel* KisDetailsPane::model() const
{
    return &d->m_model;
}
