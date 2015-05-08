%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class ViewManager : public QObject /NoDefaultCtors/ 
{
%TypeHeaderCode
#include "viewmanager.h"
%End

public:
    QAction *createAction(const QString &text);

};
