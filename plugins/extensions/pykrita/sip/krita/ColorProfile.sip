class ColorProfile : QObject
{
%TypeHeaderCode
#include "ColorProfile.h"
%End
    ColorProfile(const ColorProfile & __0);
public:
    ColorProfile(QObject*  parent /TransferThis/ = 0);
    virtual ~ColorProfile();
    QString name() const;
    void setname(QString value);
public Q_SLOTS:
Q_SIGNALS:
private:
};
