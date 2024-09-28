/**
 * @file
 *
 * @brief   Adapter implementation for the JsonCpp parser library.
 *
 * Include this file in your program to enable support for JsonCpp.
 *
 * This file defines the following classes (not in this order):
 *  - JsonCppAdapter
 *  - JsonCppArray
 *  - JsonCppArrayValueIterator
 *  - JsonCppFrozenValue
 *  - JsonCppObject
 *  - JsonCppObjectMember
 *  - JsonCppObjectMemberIterator
 *  - JsonCppValue
 *
 * Due to the dependencies that exist between these classes, the ordering of
 * class declarations and definitions may be a bit confusing. The best place to
 * start is JsonCppAdapter. This class definition is actually very small,
 * since most of the functionality is inherited from the BasicAdapter class.
 * Most of the classes in this file are provided as template arguments to the
 * inherited BasicAdapter class.
 */

#pragma once

#include <cstdint>
#include <string>
#include <iterator>

#include <json/json.h>

#include <valijson/internal/adapter.hpp>
#include <valijson/internal/basic_adapter.hpp>
#include <valijson/internal/frozen_value.hpp>
#include <valijson/exceptions.hpp>

namespace valijson {
namespace adapters {
namespace mut {

template <typename Value> class JsonCppAdapterT;
template <typename Value> class JsonCppArrayValueIterator;
template <typename Value> class JsonCppObjectMemberIterator;

template <typename Value>
using JsonCppObjectMember = std::pair<std::string, JsonCppAdapterT<Value>>;

using JsonCppAdapter = JsonCppAdapterT<Json::Value const>;
using MutableJsonCppAdapter = JsonCppAdapterT<Json::Value>;

/**
 * @brief  Light weight wrapper for a JsonCpp array value.
 *
 * This class is light weight wrapper for a JsonCpp array. It provides a
 * minimum set of container functions and typedefs that allow it to be used as
 * an iterable container.
 *
 * An instance of this class contains a single reference to the underlying
 * JsonCpp value, assumed to be an array, so there is very little overhead
 * associated with copy construction and passing by value.
 */
template <typename Value> class JsonCppArray
{
public:

    typedef JsonCppArrayValueIterator<Value> const_iterator;
    typedef JsonCppArrayValueIterator<Value> iterator;

    /// Construct a JsonCppArray referencing an empty array.
    JsonCppArray()
      : m_value() { }

    /**
     * @brief   Construct a JsonCppArray referencing a specific JsonCpp value.
     *
     * @param   value   reference to a JsonCpp value
     *
     * Note that this constructor will throw an exception if the value is not
     * an array.
     */
    JsonCppArray(Value *value)
      : m_value(value)
    {
        if (value && !value->isArray()) {
            throwRuntimeError("Value is not an array.");
        }
    }

    /**
     * @brief   Return an iterator for the first element of the array.
     *
     * The iterator return by this function is effectively the iterator
     * returned by the underlying JsonCpp implementation.
     */
    JsonCppArrayValueIterator<Value> begin() const;

    /**
     * @brief   Return an iterator for one-past the last element of the array.
     *
     * The iterator return by this function is effectively the iterator
     * returned by the underlying JsonCpp implementation.
     */
    JsonCppArrayValueIterator<Value> end() const;

    /// Return the number of elements in the array.
    size_t size() const
    {
        return m_value ? m_value->size() : 0;
    }

    MutableJsonCppAdapter create() const;

private:

    /// Reference to the contained array
    Value *m_value;

};

/**
 * @brief  Light weight wrapper for a JsonCpp object.
 *
 * This class is light weight wrapper for a JsonCpp object. It provides a
 * minimum set of container functions and typedefs that allow it to be used as
 * an iterable container.
 *
 * An instance of this class contains a single reference to the underlying
 * JsonCpp object, assumed to be an object, so there is very little overhead
 * associated with copy construction and passing by value.
 */
template <typename Value> class JsonCppObject
{
public:

    typedef JsonCppObjectMemberIterator<Value> const_iterator;
    typedef JsonCppObjectMemberIterator<Value> iterator;

    /// Construct a JsonCppObject referencing an empty object singleton.
    JsonCppObject()
      : m_value() { }

    /**
     * @brief   Construct a JsonCppObject referencing a specific JsonCpp value.
     *
     * @param   value  reference to a JsonCpp value
     *
     * Note that this constructor will throw an exception if the value is not
     * an object.
     */
    JsonCppObject(Value *value)
      : m_value(value)
    {
        if (value && !value->isObject()) {
            throwRuntimeError("Value is not an object.");
        }
    }

    /**
     * @brief   Return an iterator for this first object member
     *
     * The iterator return by this function is effectively a wrapper around
     * the iterator value returned by the underlying JsonCpp implementation.
     */
    JsonCppObjectMemberIterator<Value> begin() const;

    /**
     * @brief   Return an iterator for an invalid object member that indicates
     *          the end of the collection.
     *
     * The iterator return by this function is effectively a wrapper around
     * the iterator value returned by the underlying JsonCpp implementation.
     */
    JsonCppObjectMemberIterator<Value> end() const;

    /**
     * @brief   Return an iterator for a member/property with the given name
     *
     * @param   propertyName   Property name
     *
     * @returns a valid iterator if found, or an invalid iterator if not found
     */
    JsonCppObjectMemberIterator<Value> find(const std::string &propertyName) const;

    /// Return the number of members in the object
    size_t size() const
    {
        return m_value ? m_value->size() : 0;
    }

    MutableJsonCppAdapter create(const std::string &propertyName) const;

private:

    /// Reference to the contained object
    Value *m_value;
};

/**
 * @brief   Stores an independent copy of a JsonCpp value.
 *
 * This class allows a JsonCpp value to be stored independent of its original
 * document. JsonCpp makes this easy to do, as it does not perform any
 * custom memory management.
 *
 * @see FrozenValue
 */
class JsonCppFrozenValue: public FrozenValue
{
public:

    /**
     * @brief  Make a copy of a JsonCpp value
     *
     * @param  source  the JsonCpp value to be copied
     */
    explicit JsonCppFrozenValue(const Json::Value &source)
      : m_value(source) { }

    FrozenValue * clone() const override
    {
        return new JsonCppFrozenValue(m_value);
    }

    void setValueInto(const Adapter &into) const override;
    bool equalTo(const Adapter &other, bool strict) const override;

private:

    /// Stored JsonCpp value
    Json::Value m_value;
};

/**
 * @brief   Light weight wrapper for a JsonCpp value.
 *
 * This class is passed as an argument to the BasicAdapter template class,
 * and is used to provide access to a JsonCpp value. This class is responsible
 * for the mechanics of actually reading a JsonCpp value, whereas the
 * BasicAdapter class is responsible for the semantics of type comparisons
 * and conversions.
 *
 * The functions that need to be provided by this class are defined implicitly
 * by the implementation of the BasicAdapter template class.
 *
 * @see BasicAdapter
 */
template <typename Value> class JsonCppValue
{
public:

    /// Construct a wrapper for the empty object singleton
    JsonCppValue()
      : m_value() { }

    /// Construct a wrapper for a specific JsonCpp value
    JsonCppValue(Value *value)
      : m_value(value) { }

    /**
     * @brief   Create a new JsonCppFrozenValue instance that contains the
     *          value referenced by this JsonCppValue instance.
     *
     * @returns pointer to a new JsonCppFrozenValue instance, belonging to the
     *          caller.
     */
    FrozenValue * freeze() const
    {
        return new JsonCppFrozenValue(m_value ? *m_value : Json::Value());
    }

    /**
     * @brief   Optionally return a JsonCppArray instance.
     *
     * If the referenced JsonCpp value is an array, this function will return a
     * std::optional containing a JsonCppArray instance referencing the
     * array.
     *
     * Otherwise it will return an empty optional.
     */
    opt::optional<JsonCppArray<Value>> getArrayOptional() const
    {
        if (m_value && m_value->isArray()) {
            return opt::make_optional(JsonCppArray<Value>(m_value));
        }

        return {};
    }

    /**
     * @brief   Retrieve the number of elements in the array
     *
     * If the referenced JsonCpp value is an array, this function will retrieve
     * the number of elements in the array and store it in the output variable
     * provided.
     *
     * @param   result  reference to size_t to set with result
     *
     * @returns true if the number of elements was retrieved, false otherwise.
     */
    bool getArraySize(size_t &result) const
    {
        if (m_value && m_value->isArray()) {
            result = m_value->size();
            return true;
        }

        return false;
    }

    void setAsArray() const
    {
        if (m_value) *m_value = Json::arrayValue;
    }

    bool getBool(bool &result) const
    {
        if (m_value && m_value->isBool()) {
            result = m_value->asBool();
            return true;
        }

        return false;
    }

    void setBool(bool value) const
    {
        if (m_value) *m_value = value;
    }

    bool getDouble(double &result) const
    {
        if (m_value && m_value->isDouble()) {
            result = m_value->asDouble();
            return true;
        }

        return false;
    }

    void setDouble(double value) const
    {
        if (m_value) *m_value = value;
    }

    bool getInteger(int64_t &result) const
    {
        if (m_value && m_value->isIntegral()) {
            result = static_cast<int64_t>(m_value->asInt());
            return true;
        }

        return false;
    }

    void setInteger(int64_t value) const
    {
        if (m_value) *m_value = value;
    }

    /**
     * @brief   Optionally return a JsonCppObject instance.
     *
     * If the referenced JsonCpp value is an object, this function will return a
     * std::optional containing a JsonCppObject instance referencing the
     * object.
     *
     * Otherwise it will return an empty optional.
     */
    opt::optional<JsonCppObject<Value>> getObjectOptional() const
    {
        if (m_value && m_value->isObject()) {
            return opt::make_optional(JsonCppObject<Value>(m_value));
        }

        return {};
    }

    /**
     * @brief   Retrieve the number of members in the object
     *
     * If the referenced JsonCpp value is an object, this function will retrieve
     * the number of members in the object and store it in the output variable
     * provided.
     *
     * @param   result  reference to size_t to set with result
     *
     * @returns true if the number of members was retrieved, false otherwise.
     */
    bool getObjectSize(size_t &result) const
    {
        if (m_value && m_value->isObject()) {
            result = m_value->size();
            return true;
        }

        return false;
    }

    void setAsObject() const
    {
        if (m_value) *m_value = Json::objectValue;
    }

    bool getString(std::string &result) const
    {
        if (m_value && m_value->isString()) {
            result = m_value->asString();
            return true;
        }

        return false;
    }

    void setString(std::string const &value) const
    {
        if (m_value) *m_value = value;
    }

    static bool hasStrictTypes()
    {
        return true;
    }

    bool isArray() const
    {
        return m_value && m_value->isArray() && !m_value->isNull();
    }

    bool isBool() const
    {
        return m_value && m_value->isBool();
    }

    bool isDouble() const
    {
        return m_value && m_value->isDouble();
    }

    bool isInteger() const
    {
        return m_value && m_value->isIntegral() && !m_value->isBool();
    }

    bool isNull() const
    {
        return !m_value || m_value->isNull();
    }

    bool isNumber() const
    {
        return m_value && m_value->isNumeric()  && !m_value->isBool();
    }

    bool isObject() const
    {
        return m_value && m_value->isObject() && !m_value->isNull();
    }

    bool isString() const
    {
        return m_value && m_value->isString();
    }

    void setValue(Json::Value const &value) {
        if (m_value) *m_value = value;
    }

private:

    /// Reference to the contained JsonCpp value
    Value *m_value;
};

template <bool> struct OptionalMutableTrait {};
template <> struct OptionalMutableTrait<true> {
    typedef void mutable_adapter_tag;
};

/**
 * @brief   An implementation of the Adapter interface supporting JsonCpp.
 *
 * This class is defined in terms of the BasicAdapter template class, which
 * helps to ensure that all of the Adapter implementations behave consistently.
 *
 * @see Adapter
 * @see BasicAdapter
 */
template <typename Value> class JsonCppAdapterT:
    public BasicAdapter<JsonCppAdapterT<Value>,
                        JsonCppArray<Value>,
                        JsonCppObjectMember<Value>,
                        JsonCppObject<Value>,
                        JsonCppValue<Value>>,
    public OptionalMutableTrait<not std::is_const<Value>::value>
{
public:
    /// Construct a JsonCppAdapter that contains an empty object
    JsonCppAdapterT()
      : JsonCppAdapterT::BasicAdapter() { }

    /// Construct a JsonCppAdapter containing a specific JsonCpp value
    JsonCppAdapterT(Value *value)
      : JsonCppAdapterT::BasicAdapter(value) { }

    JsonCppAdapterT(Value &value)
      : JsonCppAdapterT::BasicAdapter(&value) { }
};

/**
 * @brief   Class for iterating over values held in a JSON array.
 *
 * This class provides a JSON array iterator that dereferences as an instance of
 * JsonCppAdapter representing a value stored in the array. It has been
 * implemented using the boost iterator_facade template.
 *
 * @see JsonCppArray
 */
template <typename Value> class JsonCppArrayValueIterator
{
public:
    using iterator_impl_t = decltype(std::declval<Value>().begin());
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = JsonCppAdapterT<Value>;
    using difference_type = std::ptrdiff_t;
    using pointer = DerefProxy<JsonCppAdapterT<Value>>;
    using reference = JsonCppAdapterT<Value>;

    JsonCppArrayValueIterator()
      : m_itr() { }

    /**
     * @brief   Construct a new JsonCppArrayValueIterator using an existing
     *          JsonCpp iterator.
     *
     * @param   itr  JsonCpp iterator to store
     */
    JsonCppArrayValueIterator(iterator_impl_t itr)
      : m_itr(itr) { }

    /// Returns a JsonCppAdapter that contains the value of the current element.
    JsonCppAdapterT<Value> operator*() const
    {
        return JsonCppAdapterT<Value>(&*m_itr);
    }

    DerefProxy<JsonCppAdapterT<Value>> operator->() const
    {
        return DerefProxy<JsonCppAdapterT<Value>>(**this);
    }

    /**
     * @brief   Compare this iterator against another iterator.
     *
     * Note that this directly compares the iterators, not the underlying
     * values, and assumes that two identical iterators will point to the same
     * underlying object.
     *
     * @param   rhs  iterator to compare against
     *
     * @returns true if the iterators are equal, false otherwise.
     */
    bool operator==(const JsonCppArrayValueIterator<Value> &rhs) const
    {
        return m_itr == rhs.m_itr;
    }

    bool operator!=(const JsonCppArrayValueIterator<Value> &rhs) const
    {
        return !(m_itr == rhs.m_itr);
    }

    JsonCppArrayValueIterator<Value>& operator++()
    {
        m_itr++;

        return *this;
    }

    JsonCppArrayValueIterator<Value> operator++(int)
    {
        JsonCppArrayValueIterator<Value> iterator_pre(m_itr);
        ++(*this);
        return iterator_pre;
    }

    JsonCppArrayValueIterator<Value>& operator--()
    {
        m_itr--;

        return *this;
    }

    void advance(std::ptrdiff_t n)
    {
        if (n > 0) {
            while (n-- > 0) {
                m_itr++;
            }
        } else {
            while (n++ < 0) {
                m_itr--;
            }
        }
    }

private:

    iterator_impl_t m_itr;
};

/**
 * @brief   Class for iterating over the members belonging to a JSON object.
 *
 * This class provides a JSON object iterator that dereferences as an instance
 * of JsonCppObjectMember representing one of the members of the object. It has
 * been implemented using the boost iterator_facade template.
 *
 * @see JsonCppObject
 * @see JsonCppObjectMember
 */
template <typename Value> class JsonCppObjectMemberIterator
{
public:
    using iterator_impl_t = decltype(std::declval<Value>().begin());
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = JsonCppObjectMember<Value>;
    using difference_type = std::ptrdiff_t;
    using pointer = DerefProxy<JsonCppObjectMember<Value>>;
    using reference = JsonCppObjectMember<Value>;

    /**
     * @brief   Construct an iterator from a JsonCpp iterator.
     *
     * @param   itr  JsonCpp iterator to store
     */
    JsonCppObjectMemberIterator(iterator_impl_t itr = {})
      : m_itr(itr) { }

    /**
     * @brief   Returns a JsonCppObjectMember that contains the key and value
     *          belonging to the object member identified by the iterator.
     */
    JsonCppObjectMember<Value> operator*() const
    {
        return JsonCppObjectMember<Value>(m_itr.key().asString(), &*m_itr);
    }

    DerefProxy<JsonCppObjectMember<Value>> operator->() const
    {
        return DerefProxy<JsonCppObjectMember<Value>>(**this);
    }

    /**
     * @brief   Compare this iterator with another iterator.
     *
     * Note that this directly compares the iterators, not the underlying
     * values, and assumes that two identical iterators will point to the same
     * underlying object.
     *
     * @param   rhs  Iterator to compare with
     *
     * @returns true if the underlying iterators are equal, false otherwise
     */
    bool operator==(const JsonCppObjectMemberIterator<Value> &rhs) const
    {
        return m_itr == rhs.m_itr;
    }

    bool operator!=(const JsonCppObjectMemberIterator<Value> &rhs) const
    {
        return !(m_itr == rhs.m_itr);
    }

    const JsonCppObjectMemberIterator<Value>& operator++()
    {
        m_itr++;

        return *this;
    }

    JsonCppObjectMemberIterator<Value> operator++(int)
    {
        JsonCppObjectMemberIterator<Value> iterator_pre(m_itr);
        ++(*this);
        return iterator_pre;
    }

    JsonCppObjectMemberIterator<Value> operator--()
    {
        m_itr--;

        return *this;
    }

private:

    /// Iternal copy of the original JsonCpp iterator
    iterator_impl_t m_itr;
};

inline void JsonCppFrozenValue::setValueInto(const Adapter &into) const
{
    if (auto *pinto = dynamic_cast<MutableJsonCppAdapter const *>(&into)) {
        pinto->getValueHandle().setValue(m_value);
    } else {
        into.setValue(JsonCppAdapter(m_value));
    }
}

inline bool JsonCppFrozenValue::equalTo(const Adapter &other, bool strict) const
{
    return JsonCppAdapter(m_value).equalTo(other, strict);
}

template <typename Value>
inline JsonCppArrayValueIterator<Value> JsonCppArray<Value>::begin() const
{
    return m_value ? m_value->begin() : const_iterator();
}

template <typename Value>
inline JsonCppArrayValueIterator<Value> JsonCppArray<Value>::end() const
{
    return m_value ? m_value->end() : const_iterator();
}

template <typename Value>
inline JsonCppObjectMemberIterator<Value> JsonCppObject<Value>::begin() const
{
    return m_value ? m_value->begin() : const_iterator();
}

template <typename Value>
inline JsonCppObjectMemberIterator<Value> JsonCppObject<Value>::end() const
{
    return m_value ? m_value->end() : const_iterator();
}

template <typename Value>
inline JsonCppObjectMemberIterator<Value> JsonCppObject<Value>::find(
        const std::string &propertyName) const
{
    if (m_value && m_value->isMember(propertyName)) {
        for (auto itr = m_value->begin(); itr != m_value->end(); ++itr) {
            if (itr.key() == propertyName) {
                return itr;
            }
        }
    }

    return end();
}

template <typename Value>
inline MutableJsonCppAdapter JsonCppArray<Value>::create() const
{
    if (!m_value) {
        return MutableJsonCppAdapter();
    }
    return MutableJsonCppAdapter(&m_value->append(Json::nullValue));
}

template <typename Value>
inline MutableJsonCppAdapter JsonCppObject<Value>::create(
        const std::string &propertyName) const
{
    if (!m_value) {
        return MutableJsonCppAdapter();
    }
    return MutableJsonCppAdapter(&(*m_value)[propertyName]);
}

}  // namespace mut

/// Specialisation of the AdapterTraits template struct for JsonCppAdapter.
template<typename Value>
struct AdapterTraits<valijson::adapters::mut::JsonCppAdapterT<Value>>
{
    // DocumentType is a raw pointer for compatibility with SchemaParser
    typedef Value* DocumentType;

    static std::string adapterName()
    {
        return "JsonCppMutableAdapter";
    }
};

}  // namespace adapters
}  // namespace valijson
