#include "KisTagModelProvider.h"

#include <QMap>
#include <QString>
#include <memory>

Q_GLOBAL_STATIC(KisTagModelProvider, s_instance)


struct KisTagModelProvider::Private {

    std::map<QString, std::unique_ptr<KisTagModel>> tagModelsMap;

};



KisTagModelProvider::KisTagModelProvider()
    : d(new Private())
{
}


KisTagModelProvider::~KisTagModelProvider()
{
    delete d;
}

KisTagModel* KisTagModelProvider::tagModel(const QString& resourceType)
{

    std::map<QString, std::unique_ptr<KisTagModel> >::const_iterator found = s_instance->d->tagModelsMap.find(resourceType);

    if (found == s_instance->d->tagModelsMap.end())
    {
        KisTagModel* model = new KisTagModel(resourceType);
        s_instance->d->tagModelsMap.insert(std::make_pair(resourceType, model));
        return model;
    }
    return found->second.get();
}
