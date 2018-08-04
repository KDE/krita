#ifndef KISDLGPALETTEEDITOR_H
#define KISDLGPALETTEEDITOR_H

#include <QDialog>
#include <QPointer>
#include <QPair>
#include <QScopedPointer>
#include <QHash>

class QAction;

class KoColorSet;
class KisSwatchGroup;

class Ui_WdgDlgPaletteEditor;

class KisDlgPaletteEditor : public QDialog
{
    Q_OBJECT
private:
    typedef QPair<QString, int> GroupInfoType; // first is name, second is rowCount
    struct OriginalPaletteInfo {
        QString name;
        QString filename;
        int columnCount;
        bool isGlobal;
        bool isReadOnly;
        QHash<QString, GroupInfoType> groups;
    };

public:
    explicit KisDlgPaletteEditor();
    ~KisDlgPaletteEditor();

public:
    void setPalette(KoColorSet *);
    KoColorSet* palette() const { return m_colorSet; }

    QString name() const;
    QString filename() const;
    int columnCount() const;
    bool isGlobal() const;
    bool isReadOnly() const;
    bool isModified() const;
    /**
     * @brief groupRemoved
     * @param groupName original group name
     * @return if the group is removed
     */
    bool groupRemoved(const QString &groupName) const;
    /**
     * @brief groupRenamedTo
     * @param groupName original group name
     * @return new group name if renamed; emptry string if not renamed or not
     * allowed to be renamed
     */
    QString groupRenamedTo(const QString &oriGroupName) const;
    /**
     * @brief groupRowNumber
     * @param groupName ORIGINAL group name
     * @return new group row number of the group
     */
    int groupRowNumber(const QString &groupName);

private Q_SLOTS:
    void slotDelGroup();
    void slotAddGroup();
    void slotRenGroup();

    void slotGroupChosen(const QString &groupName);
    void slotRowCountChanged(int);

private:
    QString oldNameFromNewName(const QString &newName);

private:
    QScopedPointer<Ui_WdgDlgPaletteEditor> m_ui;
    QScopedPointer<QAction> m_actAddGroup;
    QScopedPointer<QAction> m_actDelGroup;
    QScopedPointer<QAction> m_actRenGroup;
    QPointer<KoColorSet> m_colorSet;
    QScopedPointer<OriginalPaletteInfo> m_original;
    QHash<QString, GroupInfoType> m_groups; // first is original group name
    QString m_currentGroupOriginalName;
};

#endif // KISDLGPALETTEEDITOR_H
