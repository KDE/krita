#ifndef KISDLGPALETTEEDITOR_H
#define KISDLGPALETTEEDITOR_H

#include <QDialog>
#include <QPointer>
#include <QPair>
#include <QScopedPointer>
#include <QHash>
#include <QSet>

class QAction;

class KoColorSet;
class KisSwatchGroup;

class Ui_WdgDlgPaletteEditor;

class KisDlgPaletteEditor : public QDialog
{
    Q_OBJECT
private:
    struct GroupInfo {
        GroupInfo() { }
        GroupInfo(const QString &n, int r)
            : newName(n)
            , rowNumber(r)
        { }
        QString newName;
        int rowNumber;
    };
    struct OriginalPaletteInfo {
        QString name;
        QString filename;
        int columnCount;
        bool isGlobal;
        bool isReadOnly;
        QHash<QString, GroupInfo> groups;
    };

public:
    explicit KisDlgPaletteEditor();
    ~KisDlgPaletteEditor();

public:
    void setPalette(KoColorSet *);
    KoColorSet* palette() const { return m_colorSet; }

    QString name() const;
    QString filename() const;
    const QSet<QString> &newGroupNames() const;

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
    int groupRowNumber(const QString &groupName) const;
    bool groupKeepColors(const QString &groupName) const;

private Q_SLOTS:
    void slotDelGroup();
    void slotAddGroup();
    void slotRenGroup();

    void slotGroupChosen(const QString &groupName);
    void slotRowCountChanged(int);

private:
    QString oldNameFromNewName(const QString &newName) const;

private:
    QScopedPointer<Ui_WdgDlgPaletteEditor> m_ui;
    QScopedPointer<QAction> m_actAddGroup;
    QScopedPointer<QAction> m_actDelGroup;
    QScopedPointer<QAction> m_actRenGroup;
    QPointer<KoColorSet> m_colorSet;
    QScopedPointer<OriginalPaletteInfo> m_original;
    QHash<QString, GroupInfo> m_groups; // key is original group name
    QSet<QString> m_newGroups;
    QSet<QString> m_keepColorGroups;
    QString m_currentGroupOriginalName;

private:
    friend bool operator ==(const GroupInfo &lhs, const GroupInfo &rhs);
};

#endif // KISDLGPALETTEEDITOR_H
