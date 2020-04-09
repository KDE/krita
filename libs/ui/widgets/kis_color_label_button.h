#ifndef KISCOLORLABELBUTTON_H
#define KISCOLORLABELBUTTON_H

#include <QButtonGroup>
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

    virtual void nextCheckState() override;

Q_SIGNALS:
    void visibilityChanged(QAbstractButton* btn, bool isVisible);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


class KRITAUI_EXPORT KisColorLabelButtonGroup : public QButtonGroup {
    Q_OBJECT
public:
    KisColorLabelButtonGroup(QObject* parent);
    ~KisColorLabelButtonGroup();

    int viableButtonsChecked();
};

#endif // KISCOLORLABELBUTTON_H
