/* This file is part of the KDE project
   Copyright (C) 2005 Peter Simonsson <psn@linux.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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

    m_previewLabel->installEventFilter(this);
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
    if (watched == m_previewLabel) {
        if (e->type() == QEvent::MouseButtonDblClick) {
            openFile();
        }
    }

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
