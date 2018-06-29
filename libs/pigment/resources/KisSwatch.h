#ifndef KISSWATCH_H
#define KISSWATCH_H

#include <KoColorSetEntry.h>

#include <kritapigment_export.h>

class KRITAPIGMENT_EXPORT KisSwatch : public KoColorSetEntry
{
public:
    KisSwatch() : KoColorSetEntry() { }
    KisSwatch(const KoColor &color, const QString &name)
        : KoColorSetEntry(color, name)
    { }
    virtual ~KisSwatch() { }
public:
    KisSwatch &operator =(const KisSwatch &source);
};

#endif // KISSWATCH_H
