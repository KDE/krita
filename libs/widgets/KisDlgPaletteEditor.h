#ifndef KISDLGPALETTEEDITOR_H
#define KISDLGPALETTEEDITOR_H

#include <QDialog>
#include <QPointer>
#include <QScopedPointer>

#include <KoColorSet.h>

#include <ui_WdgDlgPaletteEditor.h>

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

private:
    QScopedPointer<Ui_WdgDlgPaletteEditor> m_ui;
    QPointer<KoColorSet> m_colorSet;
};

#endif // KISDLGPALETTEEDITOR_H
