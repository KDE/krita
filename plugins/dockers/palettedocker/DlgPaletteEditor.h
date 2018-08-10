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
class KisPaletteModel;
class KisSwatchGroup;
class KoDialog;
class KisViewManager;

class PaletteEditor;
class Ui_WdgDlgPaletteEditor;

class DlgPaletteEditor : public QDialog
{
    Q_OBJECT
public:
    explicit DlgPaletteEditor();
    ~DlgPaletteEditor();

public:
    void setPaletteModel(KisPaletteModel *);
    KoColorSet* palette() const { return m_colorSet; }
    void setView(KisViewManager *);

    void uploadChange();

private Q_SLOTS:
    void slotDelGroup();
    void slotAddGroup();
    void slotRenGroup();

    void slotGroupChosen(const QString &groupName);

    void slotRowCountChanged(int);
    void slotSetGlobal(int);
    void slotSetReadOnly(int);

    void slotNameChanged();
    void slotFilenameChanged(const QString &newFilename);
    void slotFilenameInputFinished();
    void slotColCountChanged(int);

private:
    QString oldNameFromNewName(const QString &newName) const;

private:
    QScopedPointer<Ui_WdgDlgPaletteEditor> m_ui;
    QScopedPointer<QAction> m_actAddGroup;
    QScopedPointer<QAction> m_actDelGroup;
    QScopedPointer<QAction> m_actRenGroup;
    QScopedPointer<PaletteEditor> m_paletteEditor;
    QPointer<KoColorSet> m_colorSet;
    QString m_currentGroupOriginalName;

    QPalette m_normalPalette;
    QPalette m_warnPalette;
};

#endif // KISDLGPALETTEEDITOR_H
