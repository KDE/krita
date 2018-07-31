#ifndef PALETTELISTSAVER_H
#define PALETTELISTSAVER_H

#include <QObject>

class PaletteDockerDock;
class KoColorSet;

class PaletteListSaver : public QObject
{
    Q_OBJECT
public:
    PaletteListSaver(PaletteDockerDock *dockerDock, QObject *parent = Q_NULLPTR);

private Q_SLOTS:
    void slotSetPaletteList();
    void slotSavingFinished();
    void slotPaletteModified(const KoColorSet *);

private:
    /**
     * @brief resetConnection
     * KisDocument won't reset modified to false after saving, so I need to
     * use the connection to trigger slotSavingFinished to manually set
     * modified to false
     */
    void resetConnection();

private:
    PaletteDockerDock *m_dockerDock;
};

#endif // PALETTELISTSAVER_H
