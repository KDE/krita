#ifndef KISPALETTEMANAGER_H
#define KISPALETTEMANAGER_H

#include <QObject>
#include <QScopedPointer>
#include <KisSwatch.h>

class KoColorSet;
class KisViewManager;
class KisSwatchGroup;

class KisPaletteManager : public QObject
{
    Q_OBJECT
public:
    explicit KisPaletteManager(QObject *parent = Q_NULLPTR);
    ~KisPaletteManager();
    void setPalette(KoColorSet *palette);
    void setView(KisViewManager *view);

    void addGroup(const QString &name, int rowNumber);
    void removeGroup(const QString &name);
    void renameGroup(const QString &oldName, const QString &newName);
    void setEntry(const KisSwatch &swatch, int col, int row, const QString &groupName);
    void removeEntry(int col, int row, const QString &groupName);
    void addEntry(const KisSwatch &swatch, const QString &groupName);

    bool isModified() const;
    const KisSwatchGroup &getModifiedGroup(const QString &originalName) const;

private Q_SLOTS:
    void slotUpdatePalette();

private:
    struct PaletteInfo;
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISPALETTEMANAGER_H
