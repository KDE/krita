#ifndef PALETTELISTSAVER_H
#define PALETTELISTSAVER_H

#include <QObject>
#include <palettedocker_dock.h>

class PaletteListSaver : public QObject
{
    Q_OBJECT
public:
    PaletteListSaver(PaletteDockerDock *dockerDock, QObject *parent = Q_NULLPTR);

public Q_SLOTS:
    slotSetPaletteList();

private:
    PaletteDockerDock *m_dockerDock;
};

#endif // PALETTELISTSAVER_H
