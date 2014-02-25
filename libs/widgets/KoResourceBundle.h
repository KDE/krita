#ifndef KORESOURCEBUNDLE_H
#define KORESOURCEBUNDLE_H

#include "KoResource.h"
#include "kowidgets_export.h"


class KOWIDGETS_EXPORT KoResourceBundle : public KoResource
{

public:
    KoResourceBundle(QString const&);
    
    /**
     * Load this resource.
     */
    bool load();

    /**
     * Save this resource.
     */
    bool save();

    /**
     * Returns a QImage representing this resource.  This image could be null. The image can
     * be in any valid QImage::Format.
     */
    QImage image() const;

    /// Returns the default file extension which should be when saving the resource
    QString defaultFileExtension() const;

private:
    QImage thumbnail;

};

#endif // KORESOURCEBUNDLE_H
