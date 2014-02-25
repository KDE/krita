#include "KoResourceBundle.h"

KoResourceBundle::KoResourceBundle(QString const& file):KoResource(file)
{

}

bool KoResourceBundle::load()
{
    setValid(true);
    return true;
}

bool KoResourceBundle::save()
{
    return true;
}


QString KoResourceBundle::defaultFileExtension() const{
    return QString(".zip");
}

QImage KoResourceBundle::image() const{
	return thumbnail;
}
