/*
 * selectopague.h -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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


#include <math.h>

#include <stdlib.h>

#include <qcursor.h>
#include <qapplication.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_selected_transaction.h>
#include <kis_cursor.h>
#include <kis_doc.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_iterator.h>
#include <kis_iterators_pixel.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_selection.h>
#include <kis_selection_manager.h>
#include "selectopaque.h"

typedef KGenericFactory<SelectOpaque> SelectOpaqueFactory;
K_EXPORT_COMPONENT_FACTORY( kritaselectopaque, SelectOpaqueFactory( "krita" ) )

SelectOpaque::SelectOpaque(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{

    if (parent->inherits("KisView")) {
        setInstance(SelectOpaqueFactory::instance());
        setXMLFile(locate("data","kritaplugins/selectopaque.rc"), true);
        m_view = dynamic_cast<KisView*>(parent);
        m_view->canvasSubject()->selectionManager()->addSelectionAction( new KAction(i18n("&Select All Opaque Pixels..."), 0, 0, this, SLOT(slotActivated()), actionCollection(), "selectopaque") );

    }
}

SelectOpaque::~SelectOpaque()
{
}

void SelectOpaque::slotActivated()
{
    KisSelectedTransaction *transaction;

    KisPaintDeviceSP layer = m_view->canvasSubject()->currentImg()->activeDevice();
    if (!layer) return;
    QApplication::setOverrideCursor(KisCursor::waitCursor());

    if (layer->image()->undo()) transaction = new KisSelectedTransaction(i18n("Select Opaque Pixels"), layer);
    // XXX: Multithread this!
    Q_INT32 x, y, w, h;
    layer->exactBounds(x, y, w, h);

    KisColorSpace * cs = layer->colorSpace();

    if(! layer->hasSelection())
        layer->selection()->clear();
    KisSelectionSP selection = layer->selection();

    KisHLineIterator hiter = layer->createHLineIterator(x, y, w, false);
    KisHLineIterator selIter = selection ->createHLineIterator(x, y, w, true);

    for (int row = 0; row < h; ++row) {
        while (!hiter.isDone()) {
            // Don't try to select transparent pixels.
            if (cs->getAlpha( hiter.rawData() ) > OPACITY_TRANSPARENT) {
                *(selIter.rawData()) = MAX_SELECTED;
            }
            ++hiter;
            ++selIter;
        }
        hiter.nextRow();
        selIter.nextRow();
    }
    QApplication::restoreOverrideCursor();
    layer->setDirty();
    layer->emitSelectionChanged();

    if (layer->image()->undo()) m_view->canvasSubject()->undoAdapter()->addCommand(transaction);

}

#include "selectopaque.moc"

