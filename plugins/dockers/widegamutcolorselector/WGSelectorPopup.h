/*
 *  SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef WGSELECTORPOPUP_H
#define WGSELECTORPOPUP_H

#include <QWidget>
#include <QTimer>

class KisVisualColorSelector;
class WGShadeSelector;
class WGSelectorWidgetBase;

class WGSelectorPopup : public QWidget
{
    Q_OBJECT
public:
    explicit WGSelectorPopup(QWidget *parent = nullptr);
    void setSelectorWidget(KisVisualColorSelector *selector);
    void setSelectorWidget(WGSelectorWidgetBase *selector);
    WGSelectorWidgetBase* selectorWidget() const;
public Q_SLOTS:
    void slotShowPopup();
protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event);
    void hideEvent(QHideEvent *event) override;

Q_SIGNALS:
    void sigPopupClosed(WGSelectorPopup *popup);
private Q_SLOTS:
    void slotInteraction(bool active);
private:
    void replaceCentranWidget(QWidget *widget);

    int m_margin {10};
    bool m_isInteracting {false};
    WGSelectorWidgetBase *m_selectorWidget {0};
    QTimer *m_hideTimer;
};

#endif // WGSELECTORPOPUP_H
