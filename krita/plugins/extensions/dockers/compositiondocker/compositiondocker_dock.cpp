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

#include <KoIcon.h>

#include <KoCanvasBase.h>
#include <kis_view2.h>
#include <kis_canvas2.h>
#include "compositionmodel.h"

CompositionDockerDock::CompositionDockerDock( ) : QDockWidget(i18n("Compositions")), m_canvas(0)
{
    QWidget* widget = new QWidget(this);
    setupUi(widget);
    m_model = new CompositionModel(this);
    compositionView->setModel(m_model);
    deleteButton->setIcon(koIcon("edit-delete"));
    saveButton->setIcon(koIcon("document-save"));

    setWidget(widget);

    connect( compositionView, SIGNAL(clicked( const QModelIndex & ) ),
            this, SLOT(activated ( const QModelIndex & ) ) );

    connect( deleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteClicked()));
    connect( saveButton, SIGNAL(clicked(bool)), this, SLOT(saveClicked()));
#if QT_VERSION >= 0x040700
    saveNameEdit->setPlaceholderText(i18n("Insert Name"));
#endif
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
