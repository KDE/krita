#ifndef KISPALETTECHOOSER_H
#define KISPALETTECHOOSER_H

#include <QWidget>
#include "ui_WdgPaletteChooser.h"
#include "kritaui_export.h"

class KRITAUI_EXPORT KisPaletteChooser : public QWidget
{
    Q_OBJECT
public:
    explicit KisPaletteChooser(QWidget *parent = nullptr);

Q_SIGNALS:

public Q_SLOTS:

private:
    Ui_WdgPaletteChooser *m_wdgPaletteChooser;
};

#endif // KISPALETTECHOOSER_H
