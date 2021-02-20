/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KO_ANCHOR_SELECTION_WIDGET_TEST_H
#define __KO_ANCHOR_SELECTION_WIDGET_TEST_H

#include <QtTest>

#include <KoAnchorSelectionWidget.h>

class KoAnchorSelectionWidgetTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();

private Q_SLOTS:
    void slotValueChanged(KoFlake::AnchorPosition id);
};

#endif /* __KO_ANCHOR_SELECTION_WIDGET_TEST_H */
