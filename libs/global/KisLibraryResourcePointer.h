#ifndef KISLIBRARYRESOURCEPOINTER_H
#define KISLIBRARYRESOURCEPOINTER_H

#include <utility>

namespace resource_detail {
template <typename T>
void freeResource(T*);
}


/**
 * A wrapper class for resources returned by C-style libraries. Define
 * a function `namespace resource_detail { void freeResource(Res *res) {} }
 * for every supported resource type and wrap the resource with
 * toLibraryResource() on construction. This resource will be released
 * in RAII style on destruction.
 */
template <typename T>
struct KisLibraryResourcePointer
{
    KisLibraryResourcePointer() = default;

    explicit KisLibraryResourcePointer(T *resource)
        : m_resource(resource)
    {
    }

    KisLibraryResourcePointer(KisLibraryResourcePointer &&rhs) {
        std::swap(rhs.m_resource, m_resource);
    }

    KisLibraryResourcePointer& operator=(KisLibraryResourcePointer &&rhs) {
        std::swap(rhs.m_resource, m_resource);
        return *this;
    }

    KisLibraryResourcePointer(const KisLibraryResourcePointer &rhs) = delete;
    KisLibraryResourcePointer& operator=(const KisLibraryResourcePointer &rhs) = delete;

    ~KisLibraryResourcePointer();

    T* data() {
        return m_resource;
    }

    const T* data() const {
        return m_resource;
    }

    T* operator->() {
        return m_resource;
    }

    const T* operator->() const {
        return m_resource;
    }

    operator bool() const {
        return m_resource;
    }

    /**
     * Some libraries, like FreeType use out-parameters
     * for creation of the resources. Use this pointer for
     * such libraries and the resource will be adopted
     * by the pointer automatically.
     */
    T** externalInitialization() {
        return &m_resource;
    }

private:
    T *m_resource = 0;
};

template<typename T>
KisLibraryResourcePointer<T>::~KisLibraryResourcePointer() {
    if (m_resource) {
        resource_detail::freeResource(m_resource);
    }
}

template <typename T>
KisLibraryResourcePointer<T> toLibraryResource(T *resource) {
    return KisLibraryResourcePointer<T>(resource);
}

#endif // KISLIBRARYRESOURCEPOINTER_H
