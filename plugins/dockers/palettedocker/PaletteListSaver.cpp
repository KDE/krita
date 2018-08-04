#include "PaletteListSaver.h"

#include "palettedocker_dock.h"

#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KisDocument.h>
#include <KisViewManager.h>
#include <KisPaletteModel.h>

PaletteListSaver::PaletteListSaver(PaletteDockerDock *dockerDock, QObject *parent)
    : QObject(parent)
    , m_dockerDock(dockerDock)
{
    connect(m_dockerDock->m_model, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            SLOT(slotPaletteModified()));
}

void PaletteListSaver::slotSetPaletteList()
{
    QList<const KoColorSet *> list;
    for (const KoResource * paletteResource :
         KoResourceServerProvider::instance()->paletteServer()->resources()) {
        const KoColorSet *palette = static_cast<const KoColorSet*>(paletteResource);
        Q_ASSERT(palette);
        if (!palette->isGlobal()) {
            list.append(palette);
        }
    }
    m_dockerDock->m_view->document()->setPaletteList(list);
    slotPaletteModified();
}

void PaletteListSaver::slotPaletteModified()
{
    resetConnection();
    m_dockerDock->m_view->document()->setModified(true);
}

void PaletteListSaver::resetConnection()
{
    m_dockerDock->m_view->document()->disconnect(this);
    connect(m_dockerDock->m_view->document(), SIGNAL(sigSavingFinished()),
            SLOT(slotSavingFinished()));
}

void PaletteListSaver::slotSavingFinished()
{
    bool undoStackClean = m_dockerDock->m_view->document()->undoStack()->isClean();
    m_dockerDock->m_view->document()->setModified(!undoStackClean);
}
