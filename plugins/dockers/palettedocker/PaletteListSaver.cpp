#include "PaletteListSaver.h"

#include "palettedocker_dock.h"

#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KisDocument.h>
#include <KisViewManager.h>
#include <KisPaletteModel.h>
#include <KisPaletteListWidget.h>

PaletteListSaver::PaletteListSaver(PaletteDockerDock *dockerDock, QObject *parent)
    : QObject(parent)
    , m_dockerDock(dockerDock)
{
    connect(m_dockerDock->m_model, SIGNAL(sigPaletteModifed()),
            SLOT(slotPaletteModified()));
    connect(m_dockerDock->m_paletteChooser, SIGNAL(sigPaletteListChanged()),
            SLOT(slotSetPaletteList()));
}

void PaletteListSaver::slotSetPaletteList()
{
    QList<KoColorSet *> list;
    for (KoResource * paletteResource :
         KoResourceServerProvider::instance()->paletteServer()->resources()) {
        KoColorSet *palette = static_cast<KoColorSet*>(paletteResource);
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
    KisDocument *doc = m_dockerDock->m_view->document();
    QList<KoColorSet> list;
    for (const KoResource * paletteResource :
         KoResourceServerProvider::instance()->paletteServer()->resources()) {
        const KoColorSet *palette = static_cast<const KoColorSet*>(paletteResource);
        Q_ASSERT(palette);
        if (!palette->isGlobal()) {
            list.append(KoColorSet(*palette));
        }
    }
    doc->setModified(true);
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
    qDebug() << undoStackClean;
    m_dockerDock->m_view->document()->setModified(!undoStackClean);
}
