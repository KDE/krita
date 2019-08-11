/*
 *  Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KisActionSearchWidget.h"

#include <QDebug>
#include <QPushButton>
#include <QFrame>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QAction>
#include <QVariant>

#include <kis_global.h>

KisActionModel::KisActionModel(KActionCollection *actionCollection, QObject *parent)
    : QAbstractListModel(parent)
    , m_actionCollection(actionCollection)
{
}

// reimp from QAbstractListModel
int KisActionModel::rowCount(const QModelIndex &/*parent*/) const
{
    qDebug() << "Count" << m_actionCollection->count();
    return m_actionCollection->count();
}

int KisActionModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant KisActionModel::data(const QModelIndex &index, int role) const
{
    qDebug() << index.isValid() << index;

    QAction *action = m_actionCollection->action(index.row());

    qDebug() << action;

//    switch (role) {
//    case Qt::DisplayRole:
        return action->text();
//    case Qt::WhatsThisRole:
//        return action->whatsThis();
//    case Qt::UserRole + 1:
//        return QStringList() << action->text() << action->property("tags").toStringList();
//    default:
//        return action->objectName();
//    };
}

QVariant KisActionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

KisActionSearchModel::KisActionSearchModel(QObject *parent)
{

}

void KisActionSearchModel::setFilterText(const QString &filter)
{
    m_filter = filter;
}

bool KisActionSearchModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    return true;
}

bool KisActionSearchModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    if (!index.isValid()) return false;
    return true;

    QStringList tags = index.data(Qt::UserRole + 1).toStringList();
    bool hit = false;
    Q_FOREACH(const QString &tag, tags) {
        if (true || tag.contains(m_filter)) {
            hit = true;
            break;
        }
    }
    return hit;
}

bool KisActionSearchModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QVariant leftData = sourceModel()->data(source_left, Qt::DisplayRole);
    QVariant rightData = sourceModel()->data(source_right, Qt::DisplayRole);

    QString leftName = leftData.toString();
    QString rightName = rightData.toString();
    return QString::localeAwareCompare(leftName, rightName) < 0;
}


class KisActionSearchWidget::Private
{
public:
    KisActionSearchModel *searchModel;

};

KisActionSearchWidget::KisActionSearchWidget(KActionCollection *actionCollection, QWidget *parent)
    : QWidget(parent)
    , Ui_WdgActionSearch()
    , d(new KisActionSearchWidget::Private())
{
    setupUi(this);
    connect(bnTrigger, SIGNAL(pressed()), SIGNAL(actionTriggered()));
    connect(lstAction, SIGNAL(activated(QModelIndex)), SLOT(actionSelected(QModelIndex)));
    KisActionModel *actionModel = new KisActionModel(actionCollection, this);
    d->searchModel = new KisActionSearchModel(this);
    d->searchModel->setSourceModel(actionModel);
    lstAction->setModel(actionModel);
}

KisActionSearchWidget::~KisActionSearchWidget()
{
}

void KisActionSearchWidget::actionSelected(const QModelIndex &idx)
{
    lblWhatsThis->setText(idx.data(Qt::WhatsThisRole).toString());
}

class KisActionSearchLine::Private
{
public:
    bool popupVisible {false};
    QFrame *frame {0};
    KisActionSearchWidget *searchWidget {0};
    QHBoxLayout *frameLayout {0};
};

KisActionSearchLine::KisActionSearchLine(KActionCollection *actionCollection, QWidget *parent)
    : QLineEdit(parent)
    , d(new KisActionSearchLine::Private())
{
    d->frame = new QFrame(this);
    d->searchWidget = new KisActionSearchWidget(actionCollection, this);
    connect(d->searchWidget, SIGNAL(actionTriggered), SLOT(hidePopup()));
    d->frame->setFrameStyle(QFrame::Box |  QFrame::Plain);
    d->frame->setWindowFlags(Qt::Popup);
    d->frameLayout = new QHBoxLayout(d->frame);
    d->frameLayout->setMargin(0);
    d->frameLayout->setSizeConstraint(QLayout::SetFixedSize);
    d->frame->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    d->searchWidget->setParent(d->frame);
    d->frameLayout->addWidget(d->searchWidget);
    d->frame->setFrameStyle(Qt::Popup);
}

KisActionSearchLine::~KisActionSearchLine()
{
}

void KisActionSearchLine::showPopup()
{
    if (d->searchWidget && !d->searchWidget->isVisible()) {
        d->frame->raise();
        d->frame->show();
        adjustPosition();
    }
    else {
        hidePopup();
    }
}

void KisActionSearchLine::hidePopup()
{
    if (d->searchWidget) {
        d->frame->setVisible(false);
    }
}

void KisActionSearchLine::focusInEvent(QFocusEvent *ev)
{
    QLineEdit::focusInEvent(ev);
    showPopup();
}

void KisActionSearchLine::adjustPosition()
{
    QSize popSize = d->searchWidget->size();
    QRect popupRect(this->mapToGlobal(QPoint(0, this->size().height())), popSize);

    // Get the available geometry of the screen which contains this KisPopupButton
    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->availableGeometry(this);
    popupRect = kisEnsureInRect(popupRect, screenRect);

    d->frame->setGeometry(popupRect);
}
