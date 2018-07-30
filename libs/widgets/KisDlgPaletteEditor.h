#ifndef KISDLGPALETTEEDITOR_H
#define KISDLGPALETTEEDITOR_H

#include <QDialog>
#include <QScopedPointer>

#include <ui_WdgDlgPaletteEditor.h>

class KisDlgPaletteEditor : public QDialog
{
    Q_OBJECT
public:
    explicit KisDlgPaletteEditor();
    ~KisDlgPaletteEditor();

private:
    QScopedPointer<Ui_WdgDlgPaletteEditor> m_ui;
};

#endif // KISDLGPALETTEEDITOR_H
