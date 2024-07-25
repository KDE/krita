class Swatch
{
%TypeHeaderCode
#include "Swatch.h"
%End

%ConvertToSubClassCode
  // Need to add sub class conversion, otherwise a Swatch passed to a Signal is provided as a QObject instead of
  // a Swatch object
  // (not sure why, but explicit conversion here fix the problem)
  Swatch* mc = qobject_cast<Swatch*>(sipCpp);
  sipType = 0;
  if (mc) {
      sipType = sipType_Swatch;
  }
%End

    Swatch(const Swatch & __0);
public:
    Swatch();
    bool operator==(const Swatch &other) const;
    bool operator!=(const Swatch &other) const;

public Q_SLOTS:
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
