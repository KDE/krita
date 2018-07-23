#include "PaletteListSaver.h"
#include "palettedocker_dock.h"

PaletteListSaver::PaletteListSaver(PaletteDockerDock *dockerDock, QObject *parent)
    : QObject(parent)
    , m_dockerDock(dockerDock)
{ }
