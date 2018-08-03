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
    int rowCount() const;
    bool isGlobal() const;
    bool isReadOnly() const;
    bool isModified() const;

private Q_SLOTS:
    void slotDelGroup();
    void slotAddGroup();
    void slotGroupChosen(const QString &groupName);

private:

private:
    QScopedPointer<Ui_WdgDlgPaletteEditor> m_ui;
    QScopedPointer<QAction> m_actAddGroup;
    QScopedPointer<QAction> m_actDelGroup;
    QPointer<KoColorSet> m_colorSet;
    QScopedPointer<OriginalPaletteInfo> m_original;
    QHash<QString, GroupInfoType> m_groups;
    KisSwatchGroup *m_group;
};

#endif // KISDLGPALETTEEDITOR_H
