#ifndef KISPALETTECHOOSER_H
#define KISPALETTECHOOSER_H

#include <QWidget>
#include <ui_WdgPaletteListWidget.h>

#include "KoColorSet.h"

#include "kritawidgets_export.h"

class KisPaletteListWidgetPrivate;

class KRITAWIDGETS_EXPORT KisPaletteListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KisPaletteListWidget(QWidget *parent = nullptr);
    virtual ~KisPaletteListWidget();

Q_SIGNALS:
    void sigPaletteModified(KoColorSet*);
    void sigPaletteListChanged();

public Q_SLOTS:

private:

private Q_SLOTS:
    void slotPaletteResourceSelected(KoResource *);
    void slotAdd();
    void slotRemove();
    void slotModify();
    void slotImport();
    void slotExport();

private:
    QScopedPointer<Ui_WdgPaletteListWidget> m_ui;
    QScopedPointer<KisPaletteListWidgetPrivate> m_d;
};

#endif // KISPALETTECHOOSER_H
