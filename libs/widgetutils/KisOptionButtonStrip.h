/*
 *  SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISOPTIONBUTTONSTRIP_H
#define KISOPTIONBUTTONSTRIP_H

#include <QList>
#include <QScopedPointer>
#include <QWidget>

#include <kritawidgetutils_export.h>

class KoGroupButton;

/**
 * @brief Provides a list of consecutive tool buttons
 */
class KRITAWIDGETUTILS_EXPORT KisOptionButtonStrip : public QWidget
{
    Q_OBJECT

public:
    explicit KisOptionButtonStrip(QWidget *parent = nullptr);
    ~KisOptionButtonStrip() override;

    KoGroupButton *addButton(const QIcon &icon,
                             const QString &text = QString());
    KoGroupButton *addButton(const QString &text);
    KoGroupButton *addButton();

    KoGroupButton *button(int index) const;
    QList<KoGroupButton *> buttons() const;

    bool exclusive() const;
    void setExclusive(bool exclusive);

    KoGroupButton *checkedButton() const;
    int checkedButtonIndex() const;

Q_SIGNALS:
    void buttonToggled(KoGroupButton *button, bool checked);
    void buttonToggled(int index, bool checked);

private:
    class Private;
    QScopedPointer<Private> m_d;
};

#endif
