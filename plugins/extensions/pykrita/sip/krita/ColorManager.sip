class ColorManager : QObject
{
%TypeHeaderCode
#include "ColorManager.h"
%End
    ColorManager(const ColorManager & __0);
public:
    ColorManager(QObject*  parent /TransferThis/ = 0);
    virtual ~ColorManager();
    QString type() const;
    void setType(QString value);
    InfoObject * ocioSettings() const;
    void setOcioSettings(InfoObject*  value);
public Q_SLOTS:
Q_SIGNALS:
private:
};
