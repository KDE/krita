class ColorModel : QObject
{
%TypeHeaderCode
#include "ColorModel.h"
%End
    ColorModel(const ColorModel & __0);
public:
    ColorModel(QObject*  parent /TransferThis/ = 0);
    virtual ~ColorModel();
    QString colorModelID() const;
    void setcolorModelID(QString value);
public Q_SLOTS:
Q_SIGNALS:
private:
};
