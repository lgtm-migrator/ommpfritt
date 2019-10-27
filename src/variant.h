#pragma once

#include "color/color.h"
#include <variant>
#include "geometry/vec2.h"
#include <QString>

namespace omm
{

class AbstractPropertyOwner;

class TriggerPropertyDummyValueType
{
public:
  bool operator==(const TriggerPropertyDummyValueType&) const { return true; }
  bool operator!=(const TriggerPropertyDummyValueType&) const { return false; }
};

using variant_type = std::variant< bool, double, Color, int, AbstractPropertyOwner*,
                                   QString, size_t, TriggerPropertyDummyValueType,
                                   Vec2f, Vec2i >;

std::ostream& operator<<(std::ostream& ostream, const TriggerPropertyDummyValueType& v);
std::ostream& operator<<(std::ostream& ostream, const variant_type& v);

}  // namespace omm
