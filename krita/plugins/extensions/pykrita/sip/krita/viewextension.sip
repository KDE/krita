%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip


class ViewExtension : public QObject
{

%TypeHeaderCode
#include "viewextension.h"
#include <QDebug>
%End

public:
    explicit ViewExtension(QObject *parent  /TransferThis/ = 0);
    virtual void setup(ViewManager* _viewManager) = 0;
};
