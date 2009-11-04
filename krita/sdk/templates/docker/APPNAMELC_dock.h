#ifndef _%{APPNAMEUC}_DOCK_H_
#define _%{APPNAMEUC}_DOCK_H_

#include <QDockWidget>

class QLabel;
class KisView2;

class %{APPNAME}Dock : public QDockWidget {
    Q_OBJECT
    public:
        %{APPNAME}Dock( KisView2 *view );
private:
    QLabel* m_label;
    KisView2* m_view;
};


#endif
