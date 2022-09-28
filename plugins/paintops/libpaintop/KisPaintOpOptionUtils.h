/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISPAINTOPOPTIONUTILS_H
#define KISPAINTOPOPTIONUTILS_H

#include <boost/tti/has_type.hpp>
#include <lager/state.hpp>
#include <KisZug.h>
#include <KisCurveOptionWidget2.h>

namespace KisPaintOpOptionUtils
{

namespace detail {

BOOST_TTI_HAS_TYPE(data_type)

template <typename Data, typename... Args>
struct DataStorage
{
    DataStorage(Data &&data) : m_data(std::forward<Data>(data)) {}
    lager::state<Data, lager::automatic_tag> m_data;
};

template <bool needsConversion, typename Widget, typename Data, typename... Args>
struct WidgetWrapperConversionChecker;

template <typename Widget, typename Data, typename... Args>
struct WidgetWrapperConversionChecker<false, Widget, Data, Args...> : public DataStorage<Data, Args...>, public Widget
{
    WidgetWrapperConversionChecker(Data &&data, Args... args)
        : DataStorage<Data, Args...>(std::forward<Data>(data)),
          Widget(DataStorage<Data, Args...>::m_data, std::forward<Args>(args)...)
    {
    }
};

template <typename Widget, typename Data, typename... Args>
struct WidgetWrapperConversionChecker<true, Widget, Data, Args...> : public DataStorage<Data, Args...>, public Widget
{
    WidgetWrapperConversionChecker(Data &&data, Args... args)
        : DataStorage<Data, Args...>(std::forward<Data>(data)),
          Widget(DataStorage<Data, Args...>::m_data.zoom(kiszug::lenses::to_base<typename Widget::data_type>), std::forward<Args>(args)...)
    {
    }
};

template <bool hasDataType, typename Widget, typename Data, typename... Args>
struct WidgetWrapperDataTypeChecker;

template <typename Widget, typename Data, typename... Args>
struct WidgetWrapperDataTypeChecker<false, Widget, Data, Args...> :
        WidgetWrapperConversionChecker<false, Widget, Data, Args...>
{
    using BaseClass = WidgetWrapperConversionChecker<false, Widget, Data, Args...>;

    WidgetWrapperDataTypeChecker(Data &&data, Args&&... args)
        : BaseClass(std::forward<Data>(data), std::forward<Args>(args)...)
    {
    }
};

template <typename Widget, typename Data, typename... Args>
struct WidgetWrapperDataTypeChecker<true, Widget, Data, Args...> :
        WidgetWrapperConversionChecker<!std::is_same_v<Data, typename Widget::data_type>, Widget, Data, Args...>
{
    using BaseClass =
        WidgetWrapperConversionChecker<!std::is_same_v<Data, typename Widget::data_type>, Widget, Data, Args...>;

    WidgetWrapperDataTypeChecker(Data &&data, Args&&... args)
        : BaseClass(std::forward<Data>(data), std::forward<Args>(args)...)
    {
    }
};

template <typename Widget, typename Data, typename... Args>
struct WidgetWrapper :
    WidgetWrapperDataTypeChecker<
        has_type_data_type<Widget>::value, Widget, Data, Args...>
{
    using BaseClass = WidgetWrapperDataTypeChecker<
        has_type_data_type<Widget>::value, Widget, Data, Args...>;

    WidgetWrapper(Data &&data, Args&&... args)
        : BaseClass(std::forward<Data>(data), std::forward<Args>(args)...)
    {
    }
};

}

/**
 * Creates an option widget and lager::store for its \p Data
 * type.
 *
 * The store is initialized with the passed \p data
 * object. If the widget provides the required data
 * type via `typename Widget::data_type`, and \p Data
 * is derived from it, then the function will automatically
 * create a cursor that upcasts \p Data into
 * `typename Widget::data_type`.
 *
 * All the extra arguments \p args are forwarded into the
 * widget's constructor right after the cursor to the data.
 */
template <typename Widget, typename Data, typename... Args>
KisPaintOpOption* createOptionWidget(Data&& data, Args... args)
{
    return new detail::WidgetWrapper<Widget, Data, Args...>(std::forward<Data>(data), std::forward<Args>(args)...);
}

/**
 * Creates an option widget and lager::store for its \p Data
 * type.
 *
 * This override `typename Widget::data_type` to
 * default-construct the data and pass it to the
 * widget. If widget does not provide this type,
 * compilation will fail.
 */
template <typename Widget>
KisPaintOpOption* createOptionWidget()
{
    static_assert(detail::has_type_data_type<Widget>::value, "Widget must have 'data_type' member type defined to use the default construction method");
    return createOptionWidget<Widget>(typename Widget::data_type{});
}

/**
 * Creates an instance of KisCurveOptionWidget and a
 * lager::store for its \p Data type.
 *
 * The store is initialized with the passed \p data
 * object. The object is automatically upcasted
 * with a cursor into KisCurveOptionData creation
 * of the widget.
 *
 * All the extra arguments \p args are forwarded into the
 * widget's constructor right after the cursor to the data.
 */
template <typename Data, typename... Args>
KisPaintOpOption* createCurveOptionWidget(Data&& data, Args... args)
{
    return createOptionWidget<KisCurveOptionWidget2>(std::forward<Data>(data), std::forward<Args>(args)...);
}

}

#endif // KISPAINTOPOPTIONUTILS_H
