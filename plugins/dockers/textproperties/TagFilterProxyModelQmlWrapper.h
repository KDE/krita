/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TAGFILTERPROXYMODELQMLWRAPPER_H
#define TAGFILTERPROXYMODELQMLWRAPPER_H

#include <QObject>
#include <QAbstractItemModel>
#include <KisTagFilterResourceProxyModel.h>

/**
 * @brief The TagFilterProxyModelQmlWrapper class
 *
 * This class wraps around KisTagFilterResourceProxyModel, providing properties,
 * signal compressors and handling setting the source model.
 */
class TagFilterProxyModelQmlWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *model READ model NOTIFY modelChanged)
    Q_PROPERTY(QAbstractItemModel *tagModel READ tagModel NOTIFY tagModelChanged)

    Q_PROPERTY(QString resourceType READ resourceType WRITE setResourceType NOTIFY resourceTypeChanged)
    Q_PROPERTY(QString resourceTypeName READ resourceTypeName NOTIFY resourceTypeChanged)
    Q_PROPERTY(bool importEnabled READ importEnabled NOTIFY resourceTypeChanged)

    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(int currentTag READ currentTag WRITE tagActivated NOTIFY activeTagChanged)
    Q_PROPERTY(bool searchInTag READ searchInTag WRITE setSearchInTag NOTIFY searchInTagChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentResourceChanged)
    Q_PROPERTY(QString resourceFilename READ resourceFilename WRITE setResourceToFileName NOTIFY currentResourceChanged)
    Q_PROPERTY(KoResourceSP currentResource READ currentResource NOTIFY currentResourceChanged)
public:
    TagFilterProxyModelQmlWrapper(QObject *parent = nullptr);
    ~TagFilterProxyModelQmlWrapper();


    /// The tagfilterproxymodel
    QAbstractItemModel *model() const;

    /// Associated tagmodel
    QAbstractItemModel *tagModel() const;

    /// The current resource type.
    QString resourceType() const;

    /**
     * Setting the current resource type, this expects a
     * resource type from KisResourceTypes, as this will be
     * passed onto the initializers of the all internal resource
     * related models.
     */
    void setResourceType(const QString &type);

    /// The property translated name for the current resource type.
    /// This might assert if there's no proper translated name.
    QString resourceTypeName() const;

    /// Returns whether the current resource has a ResourceLocator.
    bool importEnabled() const;

    /// Select tag at row in tag model.
    void tagActivated(const int &row);
    /// Return row of current tag in tag model.
    int currentTag() const;

    /// The current search text on the filter model.
    QString searchText() const;
    void setSearchText(const QString &text);

    /// Toggle "search in tag".
    void setSearchInTag(const bool &newSearchInTag);

    /// Whether "search in tag is enabled".
    bool searchInTag();

    /// Add new tag
    Q_INVOKABLE void addNewTag(const QString &newTagName, const int &resourceIndex = -1);

    /// Tag the resource
    Q_INVOKABLE void tagResource(const int &tagIndex, const int &resourceIndex);

    /// Untag the resource
    Q_INVOKABLE void untagResource(const int &tagIndex, const int &resourceIndex);

    /// Get a variantlist with the tags for the given resource index.
    Q_INVOKABLE QVariantList taggedResourceModel (const int &resourceIndex) const;

    /// Is the resource tagged with the current tagindex.
    Q_INVOKABLE bool showResourceTagged(const int &tagIndex, const int &resourceIndex) const;

    /// Current resource index in tagfilterproxy.
    int currentIndex() const;

    /// Set the resource index in tagfilterproxy.
    void setCurrentIndex(const int &index);

    /// Set current resource to resource file name.
    void setResourceToFileName(const QString &filename);

    /// File name of the current resource.
    QString resourceFilename();

    /// current resource.
    KoResourceSP currentResource() const;

    /// Import a resource with file picker.
    Q_INVOKABLE void importResource();

    /// Disable the currently selected resource.
    Q_INVOKABLE void removeResource();

Q_SIGNALS:
    void modelChanged();
    void searchTextChanged();
    void activeTagChanged();
    void searchInTagChanged();
    void modelSortUpdated();
    void currentResourceChanged();
    void tagModelChanged();
    void resourceTypeChanged();

private Q_SLOTS:
    /// Slot to set the search text on the model via a signal compressor.
    /// The search can be quite slow, so the signal compressor can elevate the delay here.
    void setSearchTextOnModel();
private:

    struct Private;
    const QScopedPointer<Private> d;
};

class FontFamilyTagFilterModel: public KisTagFilterResourceProxyModel {
public:
    FontFamilyTagFilterModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;

    bool additionalResourceNameChecks(const QModelIndex &index, const KisResourceSearchBoxFilter *filter) const override;
};

#endif // TAGFILTERPROXYMODELQMLWRAPPER_H
