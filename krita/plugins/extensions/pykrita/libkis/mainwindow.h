#ifndef LIBKIS_MAINWINDOW_H
#define LIBKIS_MAINWINDOW_H

#include <QObject>
#include <krita_export.h>
class KoMainWindow;

#include <view.h>

class LIBKIS_EXPORT MainWindow : public QObject
{
    Q_OBJECT
public:
    explicit MainWindow(KoMainWindow *mainWin, QObject *parent = 0);

    QList<View*> views();
signals:

public slots:


private:

    KoMainWindow *m_mainWindow;
};

#endif // MAINWINDOW_H
