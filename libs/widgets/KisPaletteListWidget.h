#ifndef KISPALETTECHOOSER_H
#define KISPALETTECHOOSER_H

#include <QWidget>
#include <ui_WdgPaletteListWidget.h>
#include "kritawidgets_export.h"

class KRITAWIDGETS_EXPORT KisPaletteListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KisPaletteListWidget(QWidget *parent = nullptr);
    virtual ~KisPaletteListWidget();

Q_SIGNALS:

public Q_SLOTS:

private:
    QScopedPointer<Ui_WdgPaletteListWidget> m_ui;
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISPALETTECHOOSER_H
