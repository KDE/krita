#ifndef KISDLGPALETTEEDITOR_H
#define KISDLGPALETTEEDITOR_H

#include <QDialog>
#include <QScopedPointer>

#include <ui_WdgDlgPaletteEditor.h>

class KisDlgPaletteEditor : public QDialog
{
public:
    explicit KisDlgPaletteEditor();
    ~KisDlgPaletteEditor();

private:
    QScopedPointer<ui_WdgDlgPaletteEditor> m_ui;
};

#endif // KISDLGPALETTEEDITOR_H
