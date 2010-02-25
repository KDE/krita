/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2008 Jarosław Staniek <staniek@kde.org>

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

#include "combobox.h"
#include "koproperty/EditorDataModel.h"
#include "koproperty/EditorView.h"
#include "koproperty/Property.h"

#include <KColorScheme>
#include <KDebug>

using namespace KoProperty;

ComboBox::Options::Options()
 : iconProvider(0)
 , extraValueAllowed(false)
{
}

ComboBox::Options::Options(const ComboBox::Options& other)
{
    *this = other;
    if (other.iconProvider)
        iconProvider = other.iconProvider->clone();
}

ComboBox::Options::~Options()
{
    delete iconProvider;
}

ComboBox::ComboBox(const Property::ListData& listData, const Options& options, QWidget *parent)
        : KComboBox(parent)
        , m_options(options)
{
//    QHBoxLayout *l = new QHBoxLayout(this);
//    l->setMargin(0);
//    l->setSpacing(0);
//    m_edit = new KComboBox(this);
//    m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    m_edit->setMinimumHeight(5);
    //setPlainWidgetStyle(m_edit);

//    l->addWidget(m_edit);

    setEditable( m_options.extraValueAllowed );
    setInsertPolicy(QComboBox::NoInsert);
//    m_edit->setMinimumSize(10, 0); // to allow the combo to be resized to a small size
    setAutoCompletion(true);
    setContextMenuPolicy(Qt::NoContextMenu);

//    if (listData)
    setListData(listData);
//    if (property->listData()) {
  //      fillValues(property);
    //}
//not needed for combo setLeavesTheSpaceForRevertButton(true);

//    setFocusWidget(m_edit);
    connect(this, SIGNAL(activated(int)), this, SLOT(slotValueChanged(int)));
    
    setFrame(false);
/*    QList<QWidget*> children( findChildren<QWidget*>() );
    foreach (QWidget* w, children) {
        kDebug() << w->objectName() << w->metaObject()->className();
        w->setStyleSheet(QString());
    }*/
    //QComboBoxPrivateContainer
    
    
    //Set the stylesheet to a plain style
    QString styleSheet;
    KColorScheme cs(QPalette::Active);
    QColor focus = cs.decoration(KColorScheme::FocusColor).color();

    styleSheet = QString("QComboBox { \
    border: 1px solid %1; \
    border-radius: 0px; \
    padding: 0px 18px; }").arg(focus.name());
   
    setStyleSheet(styleSheet);
}

ComboBox::~ComboBox()
{
}

bool ComboBox::listDataKeysAvailable() const
{
    if (m_listData.keys.isEmpty()) {
        kWarning() << "property listData not available!";
        return false;
    }
    return true;
}

QVariant ComboBox::value() const
{
    if (!listDataKeysAvailable())
        return QVariant();

    const int idx = currentIndex();
    if (idx < 0 || idx >= (int)m_listData.keys.count() || m_listData.names[idx] != currentText().trimmed()) {
        if (!m_options.extraValueAllowed || currentText().isEmpty())
            return QVariant();
        return QVariant(currentText().trimmed());//trimmed 4 safety
    }
    return QVariant(m_listData.keys[idx]);
}

void ComboBox::setValue(const QVariant &value)
{
    if (!listDataKeysAvailable())
        return;

    if (!m_setValueEnabled)
        return;
    int idx = m_listData.keys.indexOf(value.toString());
//    kDebug(30007) << "**********" << idx << "" << value.toString();
    if (idx >= 0 && idx < count()) {
        setCurrentIndex(idx);
    }
    else {
        if (idx < 0) {
            if (m_options.extraValueAllowed) {
                setCurrentIndex(-1);
                setEditText(value.toString());
            }
            kWarning() << "NO SUCH KEY:" << value.toString()
                << "property=" << objectName();
        } else {
            QStringList list;
            for (int i = 0; i < count(); i++)
                list += itemText(i);
            kWarning() << "NO SUCH INDEX WITHIN COMBOBOX:" << idx
                << "count=" << count() << "value=" << value.toString()
                << "property=" << objectName() << "\nActual combobox contents"
                << list;
        }
        setItemText(currentIndex(), QString());
    }

    if (value.isNull())
        return;

//??    if (emitChange)
//??        emit valueChanged(this);
}

/*
void ComboBox::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
    QString txt;
    if (property()->listData()) {
        const int idx = property()->listData()->keys.indexOf(value);
        if (idx >= 0)
            txt = property()->listData()->names[ idx ];
        else if (m_edit->isEditable())
            txt = m_edit->currentText();
    } else if (m_edit->isEditable()) {
        txt = m_edit->currentText();
    }

//    Widget::drawViewer(p, cg, r, txt); //keyForValue(value));
// p->eraseRect(r);
// p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, keyForValue(value));
}*/

void ComboBox::fillValues()
{
    clear();
    //m_edit->clearContents();

//    if (!m_property)
//        return;
    if (!listDataKeysAvailable())
        return;

//    m_keys = m_property->listData()->keys;
    int index = 0;
    foreach( const QString& itemName, m_listData.names ) {
        addItem(itemName);
        if (m_options.iconProvider) {
            QIcon icon = m_options.iconProvider->icon(index);
            setItemIcon(index, icon);
        }
        index++;
    }
    KCompletion *comp = completionObject();
    comp->insertItems(m_listData.names);
    comp->setCompletionMode(KGlobalSettings::CompletionShell);
}

/*
void ComboBox::setProperty( const Property *property )
{
//    const bool b = (property() == prop);
//    m_setValueEnabled = false; //setValue() couldn't be called before fillBox()
//    Widget::setProperty(prop);
//    m_setValueEnabled = true;
//    if (!b)
//    m_property = property;
    m_listData = *property->listData();
    fillValues();
//    if (prop)
//        setValue(prop->value(), false); //now the value can be set
}*/

void ComboBox::setListData(const Property::ListData & listData)
{
    m_listData = listData;
    fillValues();
}

void ComboBox::slotValueChanged(int)
{
//    emit valueChanged(this);
    emit commitData( this );
}

void ComboBox::paintEvent( QPaintEvent * event )
{
    KComboBox::paintEvent(event);
    Factory::paintTopGridLine(this);
}

/*
void ComboBox::setReadOnlyInternal(bool readOnly)
{
    setVisibleFlag(!readOnly);
}*/


/*QString
ComboBox::keyForValue(const QVariant &value)
{
  const QMap<QString, QVariant> *list = property()->valueList();
  Property::ListData *list = property()->listData();

  if (!list)
    return QString();
  int idx = listData->keys.findIndex( value );


  QMap<QString, QVariant>::ConstIterator endIt = list->constEnd();
  for(QMap<QString, QVariant>::ConstIterator it = list->constBegin(); it != endIt; ++it) {
    if(it.data() == value)
      return it.key();
  }
  return QString();
}*/

//-----------------------

ComboBoxDelegate::ComboBoxDelegate()
{
    options.removeBorders = false;
}

QString ComboBoxDelegate::displayTextForProperty( const Property* property ) const
{
    Property::ListData *listData = property->listData();
    if (!listData)
        return property->value().toString();
    if (property->value().isNull())
        return QString();
    //kDebug() << "property->value()==" << property->value();
    const int idx = listData->keys.indexOf( property->value() );
    //kDebug() << "idx==" << idx;
    if (idx == -1) {
      if (!property->option("extraValueAllowed").toBool())
        return QString();
      else
        return property->value().toString();
    }
    return property->listData()->names[ idx ];
}

QWidget* ComboBoxDelegate::createEditor( int type, QWidget *parent, 
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    Q_UNUSED(type);
    Q_UNUSED(option);
    const EditorDataModel *editorModel
        = dynamic_cast<const EditorDataModel*>(index.model());
    Property *property = editorModel->propertyForItem(index);
    ComboBox::Options options;
    options.extraValueAllowed = property->option("extraValueAllowed", false).toBool();
    ComboBox *cb = new ComboBox(*property->listData(), options, parent);
    return cb;
}

/*void ComboBoxDelegate::paint( QPainter * painter, 
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
}*/


#include "combobox.moc"
