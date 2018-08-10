#ifndef KISPALETTEMANAGER_H
#define KISPALETTEMANAGER_H

#include <QObject>
#include <QScopedPointer>
#include <KisSwatch.h>

class KoColorSet;
class KisPaletteModel;
class KisViewManager;
class KisSwatchGroup;
class KisViewManager;

class PaletteEditor : public QObject
{
    Q_OBJECT
public:
    struct PaletteInfo;

public:
    explicit PaletteEditor(QObject *parent = Q_NULLPTR);
    ~PaletteEditor();

    void setPaletteModel(KisPaletteModel *model);
    void setView(KisViewManager *view);

    void addPalette();
    void importPalette();
    void removePalette(KoColorSet *);

    int rowNumberOfGroup(const QString &oriName) const;
    QString oldNameFromNewName(const QString &newName) const;
    bool duplicateExistsFilename(const QString &name) const;

    void rename(const QString &newName);
    void changeFilename(const QString &newName);
    void changeColCount(int);

    /**
     * @brief addGroup
     * @param name original group name
     * @param rowNumber
     * @return new group's name if change accpeted, empty string if cancelled
     */
    QString addGroup();
    /**
     * @brief removeGroup
     * @param name original group name
     * @return true if change accepted, false if cancelled
     */
    bool removeGroup(const QString &name);
    /**
     * @brief renameGroup
     * @param oldName
     * @return new name if change accpeted, empty string if cancelled
     */
    QString renameGroup(const QString &oldName);
    void changeGroupRowCount(const QString &name, int newRowCount);
    void setGlobal(bool);
    void setReadOnly(bool);

    void setEntry(const KoColor &color, const QModelIndex &index);
    void removeEntry(const QModelIndex &index);
    void modifyEntry(const QModelIndex &index);
    void addEntry(const KoColor &color);

    bool isModified() const;
    const KisSwatchGroup &getModifiedGroup(const QString &originalName) const;

    void updatePalette();

private Q_SLOTS:
    void slotGroupNameChanged(const QString &newName);
    void slotPaletteChanged();

private:
    QString newPaletteFileName();
    void setNonGlobal();
    void setGlobal();
    bool duplicateExistsGroupName(const QString &name) const;
    void uploadPaletteList() const;

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISPALETTEMANAGER_H
