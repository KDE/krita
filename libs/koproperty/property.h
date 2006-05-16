/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2004-2005 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KPROPERTY_PROPERTY_H
#define KPROPERTY_PROPERTY_H

#include <QVariant>
//Added by qt3to4:
#include <Q3ValueList>
#include <QStringList>
#include <QByteArray>
#include "koproperty_global.h"

template<class U> class Q3AsciiDict;
template<class U> class Q3AsciiDictIterator;

/*! \brief Namespace for a set of classes implementing generic properties framework.

 Main classes of this framework:
  - Property, representing a single property with it's own type and value
  - Set, a set of properties
  - Editor, a widget for displaying and editing properies provided by a Set object. 
    Every property has it's own row displayed using EditorItem object, within Editor widget. 
    Widget class provides editing feature for EditorItem objects if a user selects a given item.

 KoProperty framework also supports adding custom property types 
 and custom property editor types using CustomPropertyFactory.

 \author Cedric Pasteur <cedric.pasteur@free.fr>
 \author Alexander Dymo <cloudtemple@mskat.net>
 \author Jaroslaw Staniek <js@iidea.pl>
*/
namespace KoProperty {

class PropertyPrivate;
class CustomProperty;
class Set;

///*! Helper function to create a value list from two string lists. */
//KOPROPERTY_EXPORT QMap<QString, QVariant> createValueListFromStringLists(
//	const QStringList &keys, const QStringList &values);

/*! PropertyType.
Integers that represent the type of the property. */
enum PropertyType {
	//standard supported QVariant types
	Auto = QVariant::Invalid - 1,
	Invalid = QVariant::Invalid	/**<invalid property type*/,
	Map = QVariant::Map		/**<QMap<QString, QVariant>*/,
	List = QVariant::List		  /**<QValueList<QVariant>*/,
	String = QVariant::String	  /**<string*/,
	StringList = QVariant::StringList  /**<string list*/,
	Font = QVariant::Font		  /**<font*/,
	Pixmap = QVariant::Pixmap	  /**<pixmap*/,
	//! @todo implement QVariant::Brush
	Rect = QVariant::Rect		  /**<rectangle (x,y, width, height)*/,
	Size = QVariant::Size		  /**<size (width, height)*/,
	Color = QVariant::Color		/**<color*/,
	//! \todo implement QVariant::Palette
	//! \todo implement QVariant::ColorGroup
	//! \todo implement QVariant::IconSet
	Point = QVariant::Point		/**<point (x,y)*/,
	//! \todo implement QVariant::Image
	Integer = QVariant::Int		/**<integer*/,
	//! \todo implement QVariant::UInt
	Boolean = QVariant::Bool	   /**<boolean*/,
	Double = QVariant::Double	  /**<double*/,
	CString = QVariant::CString	   /** 8-bit string*/,
	//! @todo implement QVariant::PointArray
	//! @todo implement QVariant::Region
	//! @todo implement QVariant::Bitmap
	Cursor = QVariant::Cursor	  /**<cursor*/,
	SizePolicy = QVariant::SizePolicy  /**<size policy (horizontal, vertical)*/,
	Date = QVariant::Date		  /**<date*/,
	Time = QVariant::Time	   /**<time*/,
	DateTime = QVariant::DateTime	  /**<date and time*/,
	//! @todo implement QVariant::ByteArray
	//! @todo implement QVariant::BitArray
	//! @todo implement QVariant::KeySequence
	//! @todo implement QVariant::Pen
	//! @todo implement QVariant::Long
	//! @todo implement QVariant::LongLong
	//! @todo implement QVariant::ULongLong

	//predefined custom types
	ValueFromList = 2000		   /**<string value from a list*/,
	Symbol = 2001			  /**<unicode symbol code*/,
	FontName			/**<font name, e.g. "times new roman"*/,
	FileURL			 /**<url of a file*/,
	PictureFileURL	 /**<url of a pixmap*/,
	DirectoryURL		   /**<url of a directory*/,
	LineStyle		   /**<line style*/,

	// Child property types
	Size_Height = 3001,
	Size_Width,
	Point_X,
	Point_Y,
	Rect_X,
	Rect_Y,
	Rect_Width,
	Rect_Height,
	SizePolicy_HorData,
	SizePolicy_VerData,
	SizePolicy_HorStretch,
	SizePolicy_VerStretch,

	UserDefined = 4000		 /**<plugin defined properties should start here*/
};

/*! \brief The base class representing a single property

  It can hold a property of any type supported by QVariant. You can also create you own property
  types (see Using Custom Properties in Factory doc). As a consequence, do not subclass Property,
  use \ref CustomProperty instead. \n
  Each property stores old value to allow undo. It has a name (a QCString), a caption (i18n'ed name
  shown in Editor) and a description (also i18n'ed). \n
  It also supports setting arbitrary number of options (of type option=value).
  See Editor for a list of options, and their meaning.

	\code
	// To create a property
	property = Property(name, value, caption, description); // name is a QCString,
	// value is whatever type QVariant supports

	// To create a list property
	QStringList keys, strings;
	keys << "one" << "two" << "three";
	strings << i18n("One") << i18n("Two") << i18n("Three");
	property = Property(name, keys, strings, "two", caption);
	\endcode

	Note that you need to use QVariant(bool, int) to create a boolean property value.
	See QVariant docs for more details.

	Sometimes, for longer property captions or these with more words, e.g. "Allow Zero Size",
	it's usable to provide newline characters, e.g. "Allow Zero\nSize".
	If caption argument of the constructors contains newline characters, 
	caption() will return this text with substituted these characters with spaces.
	In such cases, captionForDisplaying() is used to get the original caption text usable 
	(with newline, if any) for displaying within a property editor.

	\author Cedric Pasteur <cedric.pasteur@free.fr>
	\author Alexander Dymo <cloudtemple@mskat.net>
	\author Jaroslaw Staniek <js@iidea.pl>
*/
class KOPROPERTY_EXPORT Property
{
	public:
		//! A contant for null property
		QT_STATIC_CONST Property null;

		typedef Q3AsciiDict<Property> Dict;
		typedef Q3AsciiDictIterator<Property> DictIterator;

		/*! Data container for properties of list type. */
		class KOPROPERTY_EXPORT ListData
		{
		public:
			/*! Data container for list-value property.
			 We will be able to choose an item from this list. */
			ListData(const QStringList& keys_, const QStringList& names_);
			ListData(const Q3ValueList<QVariant> keys_, const QStringList& names_);
			ListData();
			~ListData();

			void setKeysAsStringList(const QStringList& list);
			QStringList keysAsStringList() const;

			/*! The string list containing all possible keys for this property
			 or NULL if this is not a property of type 'list'. The values in this list are ordered,
			 so the first key element is associated with first element from
			 the 'names' list, and so on. */
			Q3ValueList<QVariant> keys;
//			QStringList keys;

//! @todo what about using QValueList<QVariant> here too?
			/*! The list of i18n'ed names that will be visible on the screen.
			 First value is referenced by first key, and so on. */
			QStringList names;

//unused for now			/*! True (the default), if the list has fixed number of possible
//unused for now			 items (keys). If this is false, user can add or enter own values. */
//unused for now			bool fixed : 1;
		};

		/*! Constructs a null property. */
		Property();

		/*! Constructs property of simple type. 
		 If \a caption contains newline characters, caption() will return \a caption with substituted 
		 these with spaces. captionForDisplaying() is used to get original caption text usable 
		 (with newline, if any) for displaying within a property editor. */
		Property(const QByteArray &name, const QVariant &value = QVariant(),
			const QString &caption = QString::null, const QString &description = QString::null,
			int type = Auto, Property* parent = 0);

		/*! Constructs property of \ref ValueFromList type. */
		Property(const QByteArray &name, const QStringList &keys, const QStringList &strings, 
			const QVariant &value = QVariant(),
			const QString &caption = QString::null, const QString &description = QString::null,
			int type = ValueFromList, Property* parent = 0);

		/*! Constructs property of \ref ValueFromList type.
		 This is overload of the above ctor added for convenience. */
		Property(const QByteArray &name, ListData* listData,
			const QVariant &value = QVariant(),
			const QString &caption = QString::null, const QString &description = QString::null,
			int type = ValueFromList, Property* parent = 0);

		/*! Constructs a deep copy of \a prop property. */
		Property(const Property &prop);

		~Property();

		/*! \return the internal name of the property (that's used in List).*/
		QByteArray name() const;

		/*! Sets the internal name of the property.*/
		void setName(const QByteArray &name);

		/*! \return the caption of the property.*/
		QString caption() const;

		/*! \return the caption text of the property for displaying. 
		 It is similar to caption() but if the property caption contains newline characters, 
		 these are not substituted with spaces. */
		QString captionForDisplaying() const;

		/*! Sets the name of the property. If the caption contains newline characters, 
		 these are replaced by spaces. You can use captionForDisplaying() 
		 to access the original caption text you passed here.*/
		void setCaption(const QString &caption);

		/*! \return the description of the property.*/
		QString description() const;

		/*! Sets the description of the property.*/
		void setDescription(const QString &description);

		/*! \return the type of the property.*/
		int type() const;

		/*! Sets the type of the property.*/
		void setType(int type);

		/*! \return the value of the property.*/
		QVariant value() const;

		/*! Gets the previous property value.*/
		QVariant oldValue() const;

		/*! Sets the value of the property.*/
		void setValue(const QVariant &value, bool rememberOldValue = true, bool useCustomProperty=true);

		/*! Resets the value of the property to the old value. 
		 @see oldValue() */
		void resetValue();

		/*! \return the qstring-to-value correspondence list of the property. 
		 used to create comboboxes-like property editors.*/
		ListData* listData() const;

		/*! Sets the qstring-to-value correspondence list of the property.
		This is used to create comboboxes-like property editors.*/
		void setListData(ListData* list);

		/*! Sets the string-to-value correspondence list of the property.
		 This is used to create comboboxes-like property editors.
		 This is overload of the above ctor added for convenience. */
		void setListData(const QStringList &keys, const QStringList &names);

		/*! Sets icon by \a name for this property. Icons are optional and are used e.g.
		 in property editor - displayed at the left hand. */
		void setIcon(const QString &icon);

		/*! \return property icon's string. Can be empty. */
		QString icon() const;

		/*! \return a list of all children for this property, or NULL of there
		 is no children for this property */
		const Q3ValueList<Property*>*  children() const;

		/*! \return a child property for \a name, or NULL if there is no property with that name. */
		Property* child(const QByteArray &name);

		/*! \return parent property for this property, or NULL if there is no parent property. */
		Property* parent() const;

		/*! \return the custom property for this property.or NULL if there was
		no custom property defined. */
		CustomProperty* customProperty() const;

		/*! Sets custom property \a prop for this property. 
		 @see CustomPropertyFactory */
		void setCustomProperty(CustomProperty *prop);

		/*! \return true if this property is null. Null properties have empty names. */
		bool isNull() const;

		/*! Equivalent to !isNull() */
		operator bool () const;

		//! \return true if this property value is changed.
		bool isModified() const;

		/*! \return true if the property is read-only. 
		 The property can be read-write but still not editable because the property 
		 set containing it may be set to read-only. 
		 By default the property is read-write.
		 See Set::isReadOnly() for more details. */
		bool isReadOnly() const;

		/*! Sets this property to be read-only. 
		 @see isReadOnly() */
		void setReadOnly(bool readOnly);

		/*! \return true if the property is visible.*/
		bool isVisible() const;

		/*! Set the visibility.*/
		void setVisible(bool visible);

		/*! \return true if the property can be saved to a stream, xml, etc.
		There is a possibility to use "GUI" properties that aren't
		stored but used only in a GUI.*/
		bool isStorable() const;

		/*! Sets "storable" flag for this property. @see isStorable() */
		void setStorable(bool storable);

		/*! \return 1 if the property should be synced automatically in Property Editor
		as soon as editor contents change (e.g. when the user types text). 
		If autoSync() == 0, property value will be updated when the user presses Enter 
		or when another editor gets the focus.
		Property follows Property Editor's global rule if autoSync() !=0 and !=1 (the default).
		*/
		int autoSync() const;

		/*! if \a sync is 1, the property will be synced automatically in the Property Editor
		as soon as editor's contents change (e.g. when the user types text). 
		If \a sync is 0, property value will be updated when the user presses 
		Enter or when another editor gets the focus.
		Property follows Property Editor's global rule if sync !=0 and !=1 (the default).
		*/
		void setAutoSync(int sync);

		/*! Sets value \a val for option \a name.
		 Options are used to describe additional details for property behaviour,
		 e.g. within Editor. See Editor ctor documentation for
		 the list of supported options.
		*/
		void setOption(const char* name, const QVariant& val);

		/*! \return a value for option \a name or null value if there is no such option set. */
		QVariant option(const char* name) const;

		/*! \return true if at least one option is defined for this property. */
		bool hasOptions() const;

		/*! Equivalent to setValue(const QVariant &) */
		const Property& operator= (const QVariant& val);

		/*! Assigns a deep copy of all attributes of \a property to this property. */
		const Property& operator= (const Property &property);

		/*! Compares two properties.*/
		bool operator ==(const Property &prop) const;

		/*! \return a key used for sorting. 
		 Usually it's set by Set::addProperty() and Property::addChild() t oa unique value,
		 so that this property can be sorted in a property editor in original order. 
		 \see EditorItem::compare() */
		int sortingKey() const;

	protected:
		/*! Adds \a prop as a child of this property.
		 The children will be owned by this property. */
		void addChild(Property *prop);

		/*! Adds \a set to this property. */
		void addSet(Set *set);

		/*! Sets a key used for sorting. */
		void setSortingKey(int key);

		/*! \return a list of related properties for this property. */
		const Q3ValueList<Property*>* related() const;

		/*! Adds related property for this property. */
		void addRelatedProperty(Property *property);

		/*! Outputs debug string for this property. */
		void debug();

		//! @internal
		PropertyPrivate *d;

	friend class Set;
	friend class Buffer;
	friend class CustomProperty;
};

}

#endif
