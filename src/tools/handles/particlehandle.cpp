#include "tools/handles/particlehandle.h"
#include "renderers/painter.h"

namespace omm
{

ParticleHandle::ParticleHandle(Tool& tool) : Handle(tool)
{
  set_style(Status::Hovered, SolidStyle(Colors::YELLOW));
  set_style(Status::Active, SolidStyle(Colors::WHITE));
  set_style(Status::Inactive, SolidStyle(Color(Color::Model::RGBA, { 0.8, 0.8, 0.2, 1.0 })));
}

bool ParticleHandle::contains_global(const Vec2f& point) const
{
  const auto gpos = tool.transformation().apply_to_position(position);
  const double dist = (point - gpos).euclidean_norm();
  return dist < interact_epsilon();
}

void ParticleHandle::draw(Painter &renderer) const
{
  renderer.push_transformation(tool.transformation());
  const auto e = draw_epsilon();
  renderer.set_style(current_style());
  renderer.painter->drawRect(QRectF(to_qpoint(position), QSizeF(e, e)));
  renderer.pop_transformation();
}

}  // namespace omm
