#ifndef KISCOLORLABELBUTTON_H
#define KISCOLORLABELBUTTON_H

#include <QAbstractButton>

#include "kritaui_export.h"

class KRITAUI_EXPORT KisColorLabelButton : public QAbstractButton
{
    Q_OBJECT
public:
    KisColorLabelButton(QColor color, QWidget *parent = nullptr);
    ~KisColorLabelButton();

    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISCOLORLABELBUTTON_H
