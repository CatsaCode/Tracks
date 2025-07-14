#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include "../Vector.h"
#include "../sv/small_vector.h"
#include "PointDefinition.h"
#include "UnityEngine/GameObject.hpp"

#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"

#include <chrono>

#include "../bindings.h"

namespace Events {
struct AnimateTrackContext;
}

struct PropertyW;
struct PathPropertyW;

using PropertyNames = Tracks::ffi::PropertyNames;

struct TimeUnit {
  Tracks::ffi::CTimeUnit time;

  constexpr TimeUnit(Tracks::ffi::CTimeUnit time) : time(time) {}
  constexpr TimeUnit() = default;
  constexpr TimeUnit(TimeUnit const&) = default;

  [[nodiscard]] constexpr operator Tracks::ffi::CTimeUnit() const {
    return time;
  }

  // get seconds
  constexpr uint64_t get_seconds() const {
    return time._0;
  }

  // get nanoseconds
  constexpr uint64_t get_nanoseconds() const {
    return time._1;
  }

  constexpr bool operator<(TimeUnit o) const {
    return get_seconds() < o.get_seconds() ||
           (o.get_seconds() == get_seconds() && get_nanoseconds() < o.get_nanoseconds());
  }

  constexpr bool operator>(TimeUnit o) const {
    return get_seconds() > o.get_seconds() ||
           (o.get_seconds() == get_seconds() && get_nanoseconds() > o.get_nanoseconds());
  }

  constexpr bool operator==(TimeUnit o) const {
    return get_seconds() == o.get_seconds() && get_nanoseconds() == o.get_nanoseconds();
  }

  constexpr bool operator!=(TimeUnit o) const {
    return get_seconds() != o.get_seconds() || get_nanoseconds() != o.get_nanoseconds();
  }

  constexpr bool operator<=(TimeUnit o) const {
    return o == *this || *this < o;
  }

  constexpr bool operator>=(TimeUnit o) const {
    return o == *this || *this > o;
  }
};

struct PropertyW {
  Tracks::ffi::ValueProperty* property;

  constexpr PropertyW() = default;
  constexpr PropertyW(Tracks::ffi::ValueProperty* property) : property(property) {}

  operator Tracks::ffi::ValueProperty const*() const {
    return property;
  }
  operator Tracks::ffi::ValueProperty*() const {
    return property;
  }
  operator bool() const {
    return property != nullptr;
  }

  [[nodiscard]] Tracks::ffi::WrapBaseValueType GetType() const {
    CRASH_UNLESS(property);

    return Tracks::ffi::property_get_type(property);
  }
  [[nodiscard]] Tracks::ffi::CValueProperty GetValue() const {
    CRASH_UNLESS(property);
    return Tracks::ffi::property_get_value(property);
  }

  [[nodiscard]] TimeUnit GetTime() const {
    CRASH_UNLESS(property);

    return Tracks::ffi::property_get_last_updated(property);
  }

  constexpr bool hasUpdated(Tracks::ffi::CValueProperty value, TimeUnit lastCheckedTime = {}) const {
    return lastCheckedTime == TimeUnit() || TimeUnit(value.last_updated) >= lastCheckedTime;
  }

  [[nodiscard]] std::optional<NEVector::Quaternion> GetQuat(TimeUnit lastCheckedTime = {}) const {
    auto value = GetValue();
    if (!value.value.has_value) {
      return std::nullopt;
    }
    if (!hasUpdated(value, lastCheckedTime)) {
      return std::nullopt;
    }
    if (value.value.value.ty != Tracks::ffi::WrapBaseValueType::Quat) {
      return std::nullopt;
    }
    auto v = value.value.value.value;
    return NEVector::Quaternion{ v.quat.x, v.quat.y, v.quat.z, v.quat.w };
  }
  [[nodiscard]] std::optional<NEVector::Vector3> GetVec3(TimeUnit lastCheckedTime = {}) const {
    auto value = GetValue();
    if (!value.value.has_value) return std::nullopt;
    if (!hasUpdated(value, lastCheckedTime)) return std::nullopt;
    if (value.value.value.ty != Tracks::ffi::WrapBaseValueType::Vec3) return std::nullopt;

    auto v = value.value.value.value;
    return NEVector::Vector3{ v.vec3.x, v.vec3.y, v.vec3.z };
  }
  [[nodiscard]] std::optional<NEVector::Vector4> GetVec4(TimeUnit lastCheckedTime = {}) const {
    auto value = GetValue();
    if (!value.value.has_value) return std::nullopt;
    if (!hasUpdated(value, lastCheckedTime)) return std::nullopt;
    if (value.value.value.ty != Tracks::ffi::WrapBaseValueType::Vec4) return std::nullopt;
    auto v = value.value.value.value;

    return NEVector::Vector4{ v.vec4.x, v.vec4.y, v.vec4.z, v.vec4.w };
  }
  [[nodiscard]] std::optional<float> GetFloat(TimeUnit lastCheckedTime = {}) const {
    auto value = GetValue();
    if (!value.value.has_value) return std::nullopt;
    if (!hasUpdated(value, lastCheckedTime)) return std::nullopt;
    if (value.value.value.ty != Tracks::ffi::WrapBaseValueType::Float) return std::nullopt;

    return value.value.value.value.float_v;
  }
};

struct PathPropertyW {
  Tracks::ffi::PathProperty* property;
  Tracks::ffi::BaseProviderContext* internal_tracks_context;

  constexpr PathPropertyW() = default;
  constexpr PathPropertyW(Tracks::ffi::PathProperty* property,
                          Tracks::ffi::BaseProviderContext* internal_tracks_context)
      : property(property), internal_tracks_context(internal_tracks_context) {}
  operator Tracks::ffi::PathProperty*() const {
    return property;
  }
  operator Tracks::ffi::PathProperty const*() const {
    return property;
  }
  operator bool() const {
    return property != nullptr;
  }

  Tracks::ffi::WrapBaseValue Interpolate(float time, bool& last) const {
    auto result = Tracks::ffi::path_property_interpolate(property, time, internal_tracks_context);
    last = result.has_value;
    return result.value;
  }
  NEVector::Vector3 InterpolateVec3(float time, bool& last) const {
    auto result = Interpolate(time, last);
    return { result.value.vec3.x, result.value.vec3.y, result.value.vec3.z };
  }
  NEVector::Vector4 InterpolateVec4(float time, bool& last) const {
    auto result = Interpolate(time, last);
    return { result.value.vec4.x, result.value.vec4.y, result.value.vec4.z, result.value.vec4.w };
  }
  NEVector::Quaternion InterpolateQuat(float time, bool& last) const {
    auto result = Interpolate(time, last);
    return { result.value.quat.x, result.value.quat.y, result.value.quat.z, result.value.quat.w };
  }

  float InterpolateLinear(float time, bool& last) const {
    auto result = Interpolate(time, last);
    return result.value.float_v;
  }

  [[nodiscard]] Tracks::ffi::WrapBaseValueType GetType() const {
    return Tracks::ffi::path_property_get_type(property);
  }

  [[nodiscard]] float GetTime() const {
    return Tracks::ffi::path_property_get_time(property);
  }

  void SetTime(float time) const {
    Tracks::ffi::path_property_set_time(property, time);
  }

  void Finish() const {
    Tracks::ffi::path_property_finish(property);
  }

  void Init(std::optional<PointDefinitionW> newPointData) const {
    Tracks::ffi::path_property_init(property, newPointData.value_or(PointDefinitionW(nullptr)));
  }
};

struct TrackW {
  Tracks::ffi::Track* track;
  Tracks::ffi::TracksContext* internal_tracks_context;
  bool v2;

  constexpr TrackW() = default;
  constexpr TrackW(Tracks::ffi::Track* track, bool v2, Tracks::ffi::TracksContext* internal_tracks_context)
      : track(track), v2(v2), internal_tracks_context(internal_tracks_context) {}

  operator Tracks::ffi::Track*() const {
    return track;
  }

  operator bool() const {
    return track != nullptr;
  }

  [[nodiscard]] PropertyW GetProperty(std::string_view name) const {
    auto prop = Tracks::ffi::track_get_property(track, name.data());
    return PropertyW(prop);
  }
  [[nodiscard]] PropertyW GetPropertyNamed(Tracks::ffi::PropertyNames name) const {
    auto prop = Tracks::ffi::track_get_property_by_name(track, name);
    return PropertyW(prop);
  }

  [[nodiscard]] PathPropertyW GetPathProperty(std::string_view name) const {
    auto prop = Tracks::ffi::track_get_path_property(track, name.data());
    return PathPropertyW(prop, Tracks::ffi::tracks_context_get_base_provider_context(internal_tracks_context));
  }
  [[nodiscard]] PathPropertyW GetPathPropertyNamed(Tracks::ffi::PropertyNames name) const {
    auto prop = Tracks::ffi::track_get_path_property_by_name(track, name);
    return PathPropertyW(prop, Tracks::ffi::tracks_context_get_base_provider_context(internal_tracks_context));
  }

  void RegisterGameObject(UnityEngine::GameObject* gameObject) const {
    Tracks::ffi::track_register_game_object(track, Tracks::ffi::GameObject{ .ptr = gameObject });
  }

  // very nasty
  void* RegisterGameObjectCallback(std::function<void(UnityEngine::GameObject*, bool)> callback) const {
    if (!callback) {
      return nullptr;
    }

    // leaks memory, oh well
    auto* callbackPtr = new std::function<void(UnityEngine::GameObject*, bool)>(std::move(callback));

    // wrap the callback to a function pointer
    // this is a C-style function pointer that can be used in the FFI
    auto wrapper = +[](Tracks::ffi::GameObject gameObjectWrapper, bool isNew, void* userData) {
      auto* cb = reinterpret_cast<std::function<void(UnityEngine::GameObject*, bool)>*>(userData);
      auto gameObject = reinterpret_cast<UnityEngine::GameObject*>(const_cast<void*>(gameObjectWrapper.ptr));

      (*cb)(gameObject, isNew);
    };
    return Tracks::ffi::track_register_game_object_callback(track, wrapper, callbackPtr);
  }

  void RemoveGameObjectCallback(void* callbackPtr) const {
    if (!callbackPtr) {
      return;
    }

    auto* callback = reinterpret_cast<void (**)(Tracks::ffi::GameObject, bool)>(callbackPtr);
    Tracks::ffi::track_remove_game_object_callback(track, callback);
  }

  void RegisterProperty(std::string_view id, PropertyW property) {
    Tracks::ffi::track_register_property(track, id.data(), const_cast<Tracks::ffi::ValueProperty*>(property.property));
  }
  void RegisterPathProperty(std::string_view id, PathPropertyW property) const {
    Tracks::ffi::track_register_path_property(track, id.data(), property);
  }

  [[nodiscard]] Tracks::ffi::CPropertiesMap GetPropertiesMap() const {
    return Tracks::ffi::track_get_properties_map(track);
  }

  [[nodiscard]] Tracks::ffi::CPathPropertiesMap GetPathPropertiesMap() const {
    return Tracks::ffi::track_get_path_properties_map(track);
  }

  /**
   * @brief Get the Name object
   *
   * @return std::string_view Return a string view as the original string is leaked from the FFI.
   */
  [[nodiscard]] std::string_view GetName() const {
    return Tracks::ffi::track_get_name(track);
  }

  /**
   * @brief Set the Name object
   *
   * @param name The name to set
   */
  void SetName(std::string_view name) const {
    Tracks::ffi::track_set_name(track, name.data());
  }

  [[nodiscard]] std::span<UnityEngine::GameObject* const> GetGameObjects() const {
    static_assert(sizeof(UnityEngine::GameObject*) == sizeof(Tracks::ffi::GameObject),
                  "Tracks wrapper and GameObject pointer do not match size!");
    std::size_t count = 0;
    auto const* ptr = Tracks::ffi::track_get_game_objects(track, &count);
    auto const* castedPtr = reinterpret_cast<UnityEngine::GameObject* const*>(ptr);

    return std::span<UnityEngine::GameObject* const>(castedPtr, count);
  }
};
