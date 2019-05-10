#ifndef KISDIALOGSTATESAVERTEST_H
#define KISDIALOGSTATESAVERTEST_H

#include <QTest>
#include "ui_dialogsavertestwidget.h"

class KisDialogStateSaverTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSave();
    void testRestore();
};

#endif // KISDIALOGSTATESAVERTEST_H
