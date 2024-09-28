#pragma once

#include <valijson/internal/adapter.hpp>

namespace valijson {
template <typename AdapterType, typename = void>
struct AssignHelper {
    template <typename V> void operator()(V const &, adapters::Adapter const &) const {}
};

template <typename AdapterType>
struct AssignHelper<AdapterType, typename AdapterType::mutable_adapter_tag>
{
    template <typename Value>
    void operator()(Value const &adapter, adapters::Adapter const &to_value) const
    {
        if (to_value.isObject()) {
            adapter.setAsObject();
            to_value.applyToObject([&adapter](std::string const &key, adapters::Adapter const &val) {
                adapter.getObjectOptional()->create(key).setValue(val);
                return true;
            });
        } else if (to_value.isArray()) {
            adapter.setAsArray();
            to_value.applyToArray([&adapter](adapters::Adapter const &val) {
                adapter.getArrayOptional()->create().setValue(val);
                return true;
            });
        } else if (to_value.isString()) {
            adapter.setString(to_value.asString());
        } else if (to_value.isBool()) {
            adapter.setBool(to_value.asBool());
        } else if (to_value.isDouble()) {
            adapter.setDouble(to_value.asDouble());
        } else if (to_value.isInteger()) {
            adapter.setInteger(to_value.asInteger());
        }
    }
};

} // namespace valijson
