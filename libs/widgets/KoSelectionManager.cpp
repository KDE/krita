/* This file is part of the KDE project
*
* Copyright (C) 2008 Peter Penz <peter.penz19@gmail.com>
* Copyright (C) 2011 Paul Mendez <paulestebanms@gmail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with this library; see the file COPYING.LIB.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#include "KoSelectionManager.h"

//Calligra headers
#include "KoSelectionToggle.h"

//KDE headers
#include <KGlobalSettings>
#include <KIconLoader>

//Qt Headers
#include <QAbstractItemView>
#include <QModelIndex>
#include <QApplication>

KoSelectionManager::KoSelectionManager(QAbstractItemView *parent)
    : QObject(parent)
    , m_view(parent)
    , m_toggle(0)
    , m_connected(false)
    , m_appliedPointingHandCursor(false)
{
    connect(parent, SIGNAL(entered(const QModelIndex&)),
            this, SLOT(slotEntered(const QModelIndex&)));
    connect(parent, SIGNAL(viewportEntered()),
            this, SLOT(slotViewportEntered()));

    //TODO: Setting to enable selectiontoggle
    m_toggle = new KoSelectionToggle(m_view->viewport());
    m_toggle->setCheckable(true);
    m_toggle->hide();
    connect(m_toggle, SIGNAL(clicked(bool)),
            this, SLOT(setItemSelected(bool)));
    m_toggle->installEventFilter(this);

    m_view->viewport()->installEventFilter(this);
    m_view->setMouseTracking(true);
}

KoSelectionManager::~KoSelectionManager()
{
}

bool KoSelectionManager::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_view->viewport()) {
        switch (event->type()) {
        case QEvent::Leave:
            if (m_toggle) {
                m_toggle->hide();
            }
            restoreCursor();
            break;

        case QEvent::MouseButtonPress: {
            // Set the toggle invisible, if a mouse button has been pressed
            // outside the toggle boundaries. This e.g. assures, that the toggle
            // gets invisible during dragging items.
            if (m_toggle) {
                const QRect toggleBounds(m_toggle->mapToGlobal(QPoint(0, 0)), m_toggle->size());
                m_toggle->setVisible(toggleBounds.contains(QCursor::pos()));
            }
            break;
        }

        default:
            break;
        }
    }
    else if (watched == m_toggle) {
        switch (event->type()) {
        case QEvent::Enter:
            QApplication::changeOverrideCursor(Qt::ArrowCursor);
            break;

        case QEvent::Leave:
            QApplication::changeOverrideCursor(Qt::PointingHandCursor);
            break;

        default:
            break;
        }
    }

    return QObject::eventFilter(watched, event);
}

void KoSelectionManager::reset()
{
    if (m_toggle) {
        m_toggle->reset();
    }
}

void KoSelectionManager::slotEntered(const QModelIndex &index)
{
    const bool isSelectionCandidate = index.isValid() &&
                                      (QApplication::mouseButtons() == Qt::NoButton);

    restoreCursor();
    if (isSelectionCandidate && KGlobalSettings::singleClick()) {
        applyPointingHandCursor();
    }

    if (isSelectionCandidate) {
        if (!m_connected) {
            connect(m_view->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                    this, SLOT(slotRowsRemoved(const QModelIndex&, int, int)));
            connect(m_view->selectionModel(),
                    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                    this,
                    SLOT(slotSelectionChanged(const QItemSelection&, const QItemSelection&)));
            m_connected = true;
        }
    }
    else {
        disconnect(m_view->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                   this, SLOT(slotRowsRemoved(const QModelIndex&, int, int)));
        disconnect(m_view->selectionModel(),
                   SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                   this,
                   SLOT(slotSelectionChanged(const QItemSelection&, const QItemSelection&)));
        m_connected = false;
    }

    if (!m_toggle) {
        return;
    }

    m_toggle->hide();
    if (isSelectionCandidate) {
        m_toggle->setIndex(index);

        // Increase the size of the toggle for large items
        const int iconHeight = m_view->visualRect(index).height();

        int toggleSize = KIconLoader::SizeSmall;
        if (iconHeight >= KIconLoader::SizeEnormous) {
            toggleSize = KIconLoader::SizeLarge/2;
        }
        else if (iconHeight >= KIconLoader::SizeLarge) {
            toggleSize = KIconLoader::SizeSmallMedium;
        }

        // Add a small invisible margin, if the item-height is nearly
        // equal to the toggleSize (#169494).
        const QRect rect = m_view->visualRect(index);
        int margin = (rect.height() - toggleSize) / 2;
        if (margin > 4) {
            margin = 0;
        }
        toggleSize += 2 * margin;
        m_toggle->setMargin(margin);
        m_toggle->resize(toggleSize, toggleSize);
        m_toggle->move(rect.topLeft());

        QItemSelectionModel *selModel = m_view->selectionModel();
        m_toggle->setChecked(selModel->isSelected(index));
        m_toggle->show();
    }
    else {
        m_toggle->setIndex(QModelIndex());
    }
}

void KoSelectionManager::slotViewportEntered()
{
    if (m_toggle) {
        m_toggle->hide();
    }
    restoreCursor();
}

void KoSelectionManager::setItemSelected(bool selected)
{
    emit selectionChanged();

    if (m_toggle && m_toggle->index().isValid()) {
        QModelIndex index = m_toggle->index();
        if (index.isValid()) {
            QItemSelectionModel *selModel = m_view->selectionModel();
            if (selected) {
                selModel->select(index, QItemSelectionModel::Select);
            }
            else {
                selModel->select(index, QItemSelectionModel::Deselect);
            }
            selModel->setCurrentIndex(index, QItemSelectionModel::Current);
        }
    }
}

void KoSelectionManager::slotRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    if (m_toggle) {
        m_toggle->hide();
    }
    restoreCursor();
}

void KoSelectionManager::slotSelectionChanged(const QItemSelection &selected,
                                            const QItemSelection &deselected)
{
    // The selection has been changed outside the scope of the selection manager
    // (e. g. by the rubberband or the "Select All" action). Take care updating
    // the state of the toggle button.
    if (m_toggle && m_toggle->index().isValid()) {
        const QModelIndex index = m_toggle->index();
        if (index.isValid()) {
            if (selected.contains(index)) {
                m_toggle->setChecked(true);
            }

            if (deselected.contains(index)) {
                m_toggle->setChecked(false);
            }
        }
    }
}

void KoSelectionManager::applyPointingHandCursor()
{
    if (!m_appliedPointingHandCursor) {
        QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
        m_appliedPointingHandCursor = true;
    }
}

void KoSelectionManager::restoreCursor()
{
    if (m_appliedPointingHandCursor) {
        QApplication::restoreOverrideCursor();
        m_appliedPointingHandCursor = false;
    }
}

#include "KoSelectionManager.moc"
