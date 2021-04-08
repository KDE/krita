/*
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCOLORLABELBUTTON_H
#define KISCOLORLABELBUTTON_H

#include <QButtonGroup>
#include <QAbstractButton>
#include <QSet>

#include "kritaui_export.h"

class KRITAUI_EXPORT KisColorLabelButton : public QAbstractButton
{
    Q_OBJECT
public:
    enum SelectionIndicationType {
        FillIn,
        Outline
    };

    KisColorLabelButton(QColor color, uint sizeSquared = 32, QWidget *parent = nullptr);
    ~KisColorLabelButton();

    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    QSize sizeHint() const override;
    void setSelectionVisType( SelectionIndicationType type );

    virtual void nextCheckState() override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


class KRITAUI_EXPORT KisColorLabelFilterGroup : public QButtonGroup {
    Q_OBJECT
public:
    KisColorLabelFilterGroup(QObject* parent);
    ~KisColorLabelFilterGroup();

    QList<QAbstractButton*> viableButtons() const;
    void setViableLabels(const QSet<int> &buttons);
    void setViableLabels(const QList<int> &viableLabels);
    QSet<int> getActiveLabels() const;

    QList<QAbstractButton*> checkedViableButtons() const;
    int countCheckedViableButtons() const;
    int countViableButtons() const;

    void setMinimumRequiredChecked( int checkedBtns );
    int minimumRequiredChecked() const;

public Q_SLOTS:
    void reset();
    void setAllVisibility(const bool vis);

private:
    void disableAll();
    QSet<int> viableColorLabels;
    int minimumCheckedButtons;

};

class KRITAUI_EXPORT KisColorLabelMouseDragFilter : public QObject {
    enum State{
        Idle,
        WaitingForDragLeave, //Waiting for mouse to exit first clicked while the mouse button is down.
        WaitingForDragEnter //Waiting for mouse to slide across buttons within the same button group.
    };

    State currentState;
    QPoint lastKnownMousePosition;

public:
    KisColorLabelMouseDragFilter(QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void checkSlideOverNeighborButtons(QMouseEvent* mouseEvent, class QAbstractButton* startingButton);
};


#endif // KISCOLORLABELBUTTON_H
