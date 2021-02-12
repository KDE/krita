/*
 *  Author 2021 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <TestKisTagSelectionWidget.h>

#include <QTest>
#include <QDialog>

#include <QVBoxLayout>

#include "kis_debug.h"

#include <KisTagSelectionWidget.h>


void TestKisTagSelectionWidget::test()
{
    KisTagSelectionWidget* widget = new KisTagSelectionWidget();
    QList<KoID> selected = createSelectedTags();
    QList<KoID> rest = createAvailableTags();
    widget->setTagList(true, selected, rest);
    //widget->m_addTagButton->actions()

    WdgCloseableLabel* label = firstCloseableLabel(widget->m_layout);
    qCritical() << "Label is null or not: " << (label ? "not null" : "null");
    if (label) {
        QList<QAction*> actionsList = label->actions();
        qCritical() << actionsList.count();
    }






    delete widget;
}


WdgCloseableLabel* TestKisTagSelectionWidget::firstCloseableLabel(QLayout* layout)
{
    QLayoutItem* item;
    qCritical() << "layout has count: " << layout->count();
    for (int i = 0; i < layout->count(); i++) {
        item = layout->itemAt(i);
        if (item->widget()) {
            qCritical() << "ok, we have a widget " << i;
            qCritical() << item->widget()->inherits("WdgCloseableLabel");
            qCritical() << item->widget()->inherits("WdgAddTagLabel");
            qCritical() << item->widget()->inherits("WdgAddTagsCategoriesLabel");

            WdgCloseableLabel* label = dynamic_cast<WdgCloseableLabel*>(item->widget());
            if (label) {
                return label;
            }
        }
    }
    return 0;
}



QList<KoID> TestKisTagSelectionWidget::createAvailableTags()
{
    QList<KoID> response;

    //response.append(CustomTagSP(new CustomTag("tag1", QVariant(1))));
    //response.append(CustomTagSP(new CustomTag("tag2", QVariant(2))));
    //response.append(CustomTagSP(new CustomTag("tag3", QVariant(3))));
    //response.append(CustomTagSP(new CustomTag("tag4", QVariant(4))));
    //response.append(CustomTagSP(new CustomTag("tag5", QVariant(5))));
    //response.append(CustomTagSP(new CustomTag("tag6", QVariant(6))));

    return response;
}

QList<KoID> TestKisTagSelectionWidget::createSelectedTags()
{
    QList<KoID> response;
    //response.append(CustomTagSP(new CustomTag("tag11", QVariant(11))));
    //response.append(CustomTagSP(new CustomTag("tag12", QVariant(12))));
    //response.append(CustomTagSP(new CustomTag("tag13", QVariant(13))));
    //response.append(CustomTagSP(new CustomTag("tag14", QVariant(14))));

    return response;
}

QTEST_MAIN(TestKisTagSelectionWidget)
