/*
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "compositiondocker_dock.h"

#include <QGridLayout>
#include <QListView>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QInputDialog>
#include <QThread>
#include <QAction>

#include <klocale.h>
#include <KActionCollection>

#include <KoCanvasBase.h>
#include <kis_view2.h>
#include <kis_canvas2.h>
#include "compositionmodel.h"


// class KisTasksetDelegate : public QStyledItemDelegate
// {
// public:
//     KisTasksetDelegate(QObject * parent = 0) : QStyledItemDelegate(parent) {}
//     virtual ~KisTasksetDelegate() {}
//     /// reimplemented
//     QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const {
//         return QSize(QStyledItemDelegate::sizeHint(option, index).width(),
//                      qMin(QStyledItemDelegate::sizeHint(option, index).width(), 25));
//     }
// };
// 
// class KisTasksetResourceDelegate : public QStyledItemDelegate
// {
// public:
//     KisTasksetResourceDelegate(QObject * parent = 0) : QStyledItemDelegate(parent) {}
//     virtual ~KisTasksetResourceDelegate() {}
//     /// reimplemented
//     virtual void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
// };
// 
// void KisTasksetResourceDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
// {
//     if (! index.isValid())
//         return;
// 
//     TasksetResource* taskset = static_cast<TasksetResource*>(index.internalPointer());
// 
//     if (option.state & QStyle::State_Selected) {
//         painter->setPen(QPen(option.palette.highlight(), 2.0));
//         painter->fillRect(option.rect, option.palette.highlight());
//     }
// 
//     painter->setPen(Qt::black);
//     painter->drawText(option.rect.x() + 5, option.rect.y() + painter->fontMetrics().ascent() + 5, taskset->name());
// 
// }

CompositionDockerDock::CompositionDockerDock( ) : QDockWidget(i18n("Compositions")), m_canvas(0)
{
    QWidget* widget = new QWidget(this);
    setupUi(widget);
    m_model = new CompositionModel(this);
    compositionView->setModel(m_model);
//     tasksetView->setItemDelegate(new KisTasksetDelegate(this));
    deleteButton->setIcon(KIcon("edit-delete"));
    saveButton->setIcon(KIcon("document-save"));
//     saveButton->setEnabled(false);

    setWidget(widget);

    connect( compositionView, SIGNAL(clicked( const QModelIndex & ) ),
            this, SLOT(activated ( const QModelIndex & ) ) );

    connect( deleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteClicked()));
    connect( saveButton, SIGNAL(clicked(bool)), this, SLOT(saveClicked()));
}

CompositionDockerDock::~CompositionDockerDock()
{
    
}

void CompositionDockerDock::setCanvas(KoCanvasBase * canvas)
{
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    updateModel();
}

void CompositionDockerDock::unsetCanvas()
{
    m_canvas = 0;
    m_model->setCompositions(QList<KisLayerComposition*>());
}

void CompositionDockerDock::activated(const QModelIndex& index)
{
    KisLayerComposition* composition = m_model->compositionFromIndex(index);
    composition->apply();
}

void CompositionDockerDock::deleteClicked()
{
    QModelIndex index = compositionView->currentIndex();
    if(index.isValid()) {
        KisLayerComposition* composition = m_model->compositionFromIndex(index);
        m_canvas->view()->image()->removeComposition(composition);
        updateModel();
    }
}

void CompositionDockerDock::saveClicked()
{
    KisImageWSP image = m_canvas->view()->image();
    KisLayerComposition* composition = new KisLayerComposition(image, saveNameEdit->text());
    composition->store();
    image->addComposition(composition);
    saveNameEdit->clear();
    updateModel();
}

void CompositionDockerDock::updateModel()
{
    m_model->setCompositions(m_canvas->view()->image()->compositions());
}


#include "compositiondocker_dock.moc"
