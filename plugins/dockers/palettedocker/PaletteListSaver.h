#ifndef PALETTELISTSAVER_H
#define PALETTELISTSAVER_H

#include <QObject>

class PaletteDockerDock;

class PaletteListSaver : public QObject
{
    Q_OBJECT
public:
    PaletteListSaver(PaletteDockerDock *dockerDock, QObject *parent = Q_NULLPTR);

private Q_SLOTS:
    void slotSetPaletteList();

private:
    PaletteDockerDock *m_dockerDock;
};

#endif // PALETTELISTSAVER_H
