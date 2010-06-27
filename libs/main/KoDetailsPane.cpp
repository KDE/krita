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

#include "KoDetailsPane.h"

#include <QStandardItemModel>
#include <QKeyEvent>

#include <kcomponentdata.h>
#include <kglobalsettings.h>


////////////////////////////////////
// class KoDetailsPane
///////////////////////////////////

class KoDetailsPanePrivate
{
public:
    KoDetailsPanePrivate(const KComponentData &componentData)
            : m_componentData(componentData) {
        m_model = new QStandardItemModel;
    }
    ~KoDetailsPanePrivate() {
        delete m_model;
    }

    KComponentData m_componentData;
    QStandardItemModel* m_model;
};

KoDetailsPane::KoDetailsPane(QWidget* parent, const KComponentData &_componentData, const QString& header)
        : QWidget(parent),
        Ui_KoDetailsPaneBase(),
        d(new KoDetailsPanePrivate(_componentData))
{
    d->m_model->setHorizontalHeaderItem(0, new QStandardItem(header));

    setupUi(this);

    m_previewLabel->installEventFilter(this);
    m_documentList->installEventFilter(this);
    m_documentList->setIconSize(QSize(64, 64));
    m_documentList->setModel(d->m_model);
    m_splitter->setSizes(QList<int>() << 2 << 1);

    changePalette();

    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(changePalette()));

    connect(m_documentList->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(selectionChanged(const QModelIndex&)));
    connect(m_documentList, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(openFile(const QModelIndex&)));
    connect(m_openButton, SIGNAL(clicked()), this, SLOT(openFile()));
}

KoDetailsPane::~KoDetailsPane()
{
    delete d;
}

KComponentData KoDetailsPane::componentData()
{
    return d->m_componentData;
}

bool KoDetailsPane::eventFilter(QObject* watched, QEvent* e)
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

void KoDetailsPane::resizeSplitter(KoDetailsPane* sender, const QList<int>& sizes)
{
    if (sender == this)
        return;

    m_splitter->setSizes(sizes);
}

void KoDetailsPane::openFile()
{
    QModelIndex index = m_documentList->selectionModel()->currentIndex();
    openFile(index);
}

void KoDetailsPane::changePalette()
{
    QPalette p = palette();
    p.setBrush(QPalette::Base, QColor(Qt::transparent));
    p.setColor(QPalette::Text, p.color(QPalette::Normal, QPalette::Foreground));
    m_detailsLabel->setPalette(p);
}

QStandardItemModel* KoDetailsPane::model() const
{
    return d->m_model;
}

#include <KoDetailsPane.moc>
