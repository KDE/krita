/*
 *
 */

#ifndef KISTOOLTIPMANAGER_H
#define KISTOOLTIPMANAGER_H

#include <QObject>
#include <QMap>

class KisView2;
class KisTooltipManager : public QObject
{
    Q_OBJECT

public:
    KisTooltipManager(KisView2* view);
    ~KisTooltipManager();

    void record();

private slots:
    void captureToolip();
private:
    KisView2* m_view;
    QMap<QString, QString> m_tooltipMap;
};

#endif // KISTOOLTIPMANAGER_H
