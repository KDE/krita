/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSTRETCHEDICONBUTTON_H
#define KISSTRETCHEDICONBUTTON_H

#include <kritawidgets_export.h>
#include <QToolButton>

#include <QPointer>
#include <kis_signal_auto_connection.h>

class QLabel;
class QAction;

class KRITAWIDGETS_EXPORT KisStretchedIconButton : public QToolButton
{
public:
    KisStretchedIconButton(QWidget *parent);
    ~KisStretchedIconButton();

    void setStretchedIcon(const QIcon &icon);
    QIcon stretchedIcon() const;

    void setAssociatedAction(QAction *action);
    QAction* associatedAction() const;

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void updateLabelIcon();

protected Q_SLOTS:
    void slotActionChanged();

private:
    QLabel *m_label {nullptr};
    QPointer<QAction> m_action;
    KisSignalAutoConnectionsStore m_actionConnections;
    QIcon m_stretchedIcon;
};

#endif // KISSTRETCHEDICONBUTTON_H
