/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STORYBOARD_ITEM
#define STORYBOARD_ITEM

#include <QVariant>
#include <QVector>
#include <QPixmap>

#include "kritaui_export.h"
#include "kis_types.h"

//each storyboardItem contains pointer to child data
class StoryboardItem;
class QDomDocument;
class QDomElement;

/**
 * @struct Comment
 * @brief This class is a simple combination of two variables.
 * It stores the name and visibility of comments. It is used in
 * @c CommentModel and @c StoryboardModel.
 */
struct StoryboardComment
{
    QString name;
    bool visibility;
};

/**
 * @class CommentBox
 * @brief This class is a simple combination of two QVariants.
 * It can be converted to and from QVarinat type and
 * is used in StoryboardModel.
 */
class CommentBox
{
public:
    CommentBox()
    : content("")
    , scrollValue(0)
    {}
    CommentBox(const CommentBox& other)
    : content(other.content)
    , scrollValue(other.scrollValue)
    {}
    ~CommentBox()
    {}

    /**
     * @brief the text content of the Comment
     */
    QVariant content;
    /**
     * @brief the value of the scroll bar of the comment scrollbar
     */
    QVariant scrollValue;
};


/**
 * @class ThumbnailData
 * @brief This class is a simple combination of two QVariants.
 * It can be converted to and from QVarinat type and
 * is used in StoryboardModel.
 */
class ThumbnailData
{
public:
    ThumbnailData()
    : frameNum("")
    , pixmap(QPixmap())
    {}
    ThumbnailData(const ThumbnailData& other)
    : frameNum(other.frameNum)
    , pixmap(other.pixmap)
    {}
    ~ThumbnailData()
    {}

    /**
     * @brief the frame number corresponding to this item
     * in the timeline docker
     */
    QVariant frameNum;

    /**
     * @brief a scaled down thumbnail version of the frame
     */
    QVariant pixmap;
};

Q_DECLARE_METATYPE(CommentBox)
Q_DECLARE_METATYPE(ThumbnailData)

/**
 * @class StoryboardChild
 * @brief This class makes up the StoryboardItem
 * class. It consists of pointer to its parent item
 * and the data stored as QVariant.
 */
class StoryboardChild
{
public:
    StoryboardChild(QVariant data)
        : m_data(data)
    {}

    StoryboardChild(const StoryboardChild &rhs)
        : m_data(rhs.m_data)
    {}

    StoryboardItemSP parent()
    {
        return m_parentItem.toStrongRef();
    }

    void setParent(StoryboardItemSP parent)
    {
        m_parentItem = parent;
    }

    QVariant data()
    { 
        return m_data;
    }
    void setData(QVariant value)
    {
        m_data = value;
    }

private:
    QVariant m_data;
    QWeakPointer<StoryboardItem> m_parentItem;
};

/**
 * @class StoryboardItem
 * @brief This class stores a list of StoryboardChild objects
 * and provides functionality to manipulate the list. Specific
 * item type must be stored at specific indices
 * @param childType enum for the indices and corresponding data type to be stored.
 */
class KRITAUI_EXPORT StoryboardItem : public QEnableSharedFromThis<StoryboardItem>
{
public:
    explicit StoryboardItem();
    StoryboardItem(const StoryboardItem& other);
    ~StoryboardItem();

    void appendChild(QVariant data);
    void cloneChildrenFrom(const StoryboardItem &other);
    void insertChild(int row, QVariant data = QVariant());
    void removeChild(int row);
    void moveChild(int from, int to);
    int childCount() const;
    QSharedPointer<StoryboardChild> child(int row) const;

    QDomElement toXML(QDomDocument doc);
    void loadXML(const QDomElement &itemNode);

    static StoryboardItemList cloneStoryboardItemList(const StoryboardItemList &list);


    /**
     * @enum childType
     * @brief This enum defines the data type to be stored at particular indices
     * @param FrameNumber Store the frame number at index 0. Data type stored here should be @c ThumbnailData.
     * @param ItemName Store the item name at index 1. Data type stored here should be @c string.
     * @param DurationSecond Store the duration in second at index 2. Data type should be @c int.
     * @param DurationFrame Store the duration in frame at index 3. Data type should be @c int.
     * @param Comments Store the comments at indices @a greater_than_or_equal_to to index 4. Data type should be @c CommentBox.
     */
    enum childType{

        /**
         * @brief Store the frame number at index 0. Data type stored here should be @c ThumbnailData
         */
        FrameNumber,
        /**
         * @brief Store the item name at index 1. Data type stored here should be @c string.
         */
        ItemName,
        /**
         * @brief Store the duration in second at index 2. Data type stored here should be @c int.
         */
        DurationSecond,
        /**
         * @brief Store the duration in frame at index 3. Data type stored here should be @c int.
         */
        DurationFrame,
        /**
         * @brief Store the comments at indices @a greater_than_or_equal_to to index 4. Data type stored here should be @c CommentBox
         */
        Comments
    };

private:
    QVector<QSharedPointer<StoryboardChild>> m_childData;
};

#endif
