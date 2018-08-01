#ifndef KISDLGPALETTEEDITOR_H
#define KISDLGPALETTEEDITOR_H

#include <QDialog>
#include <QPointer>
#include <QScopedPointer>

class QAction;

class KoColorSet;

class Ui_WdgDlgPaletteEditor;

class KisDlgPaletteEditor : public QDialog
{
    Q_OBJECT
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

private Q_SLOTS:
    void slotDelGroup();
    void slotAddGroup();
    void slotToggleGlobal(int);

private:
    QScopedPointer<Ui_WdgDlgPaletteEditor> m_ui;
    QScopedPointer<QAction> m_actAddGroup;
    QScopedPointer<QAction> m_actDelGroup;
    QPointer<KoColorSet> m_colorSet;
};

#endif // KISDLGPALETTEEDITOR_H
