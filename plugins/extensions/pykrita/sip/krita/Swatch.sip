class Swatch
{
%TypeHeaderCode
#include "Swatch.h"
%End
    Swatch(const Swatch & __0);
public:
    Swatch();
    QString name() const;
    void setName(const QString &name);
    QString id() const;
    void setId(const QString &id);
    ManagedColor *color() const  /Factory/;
    void setColor(ManagedColor *color);
    bool spotColor() const;
    void setSpotColor(bool spotColor);
    bool isValid() const;

};
