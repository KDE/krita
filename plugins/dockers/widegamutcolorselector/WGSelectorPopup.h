/*
 *  SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef WGSELECTORPOPUP_H
#define WGSELECTORPOPUP_H

#include <QWidget>

class KisVisualColorSelector;

class WGSelectorPopup : public QWidget
{
    Q_OBJECT
public:
    explicit WGSelectorPopup(QWidget *parent = nullptr);
    void setSelectorWidget(KisVisualColorSelector *selector);
public Q_SLOTS:
    void slotShowPopup();
protected:
    void paintEvent(QPaintEvent *event) override;
    //void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event);

Q_SIGNALS:
private Q_SLOTS:
    void slotInteraction(bool active);
private:
    int m_margin {10};
    bool m_isInteracting {false};
};

#endif // WGSELECTORPOPUP_H
