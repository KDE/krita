#include "PaletteListSaver.h"
#include "palettedocker_dock.h"

#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KisViewManager.h>
#include <KisDocument.h>

PaletteListSaver::PaletteListSaver(PaletteDockerDock *dockerDock, QObject *parent)
    : QObject(parent)
    , m_dockerDock(dockerDock)
{
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
}
