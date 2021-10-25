#pragma once

#include "geometry/vec2.h"
#include "objects/pathiterator.h"
#include "tools/handles/handle.h"
#include "tools/handles/particlehandle.h"
#include "tools/handles/tangenthandle.h"

namespace omm
{

class Scene;
class Path;

class AbstractSelectHandle : public Handle
{
public:
  explicit AbstractSelectHandle(Tool& tool);
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event) override;
  void mouse_release(const Vec2f& pos, const QMouseEvent& event) override;
  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override;

protected:
  virtual void set_selected(bool selected) = 0;
  virtual void clear() = 0;
  [[nodiscard]] virtual bool is_selected() const = 0;
  static constexpr auto extend_selection_modifier = Qt::ShiftModifier;

private:
  bool m_was_selected = false;
};

class ObjectSelectHandle : public AbstractSelectHandle
{
public:
  explicit ObjectSelectHandle(Tool& tool, Scene& scene, Object& object);
  [[nodiscard]] bool contains_global(const Vec2f& point) const override;
  void draw(QPainter& painter) const override;

protected:
  [[nodiscard]] ObjectTransformation transformation() const;
  void set_selected(bool selected) override;
  void clear() override;
  [[nodiscard]] bool is_selected() const override;

private:
  Scene& m_scene;
  Object& m_object;
};

class PointSelectHandle : public AbstractSelectHandle
{
public:
  enum class TangentMode { Mirror, Individual };
  explicit PointSelectHandle(Tool& tool, const PathIterator& iterator);
  [[nodiscard]] bool contains_global(const Vec2f& point) const override;
  void draw(QPainter& painter) const override;
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event) override;
  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override;
  void mouse_release(const Vec2f& pos, const QMouseEvent& event) override;

  void transform_tangent(const Vec2f& delta, TangentHandle::Tangent tangent);
  bool force_draw_subhandles = false;

protected:
  [[nodiscard]] ObjectTransformation transformation() const;
  void set_selected(bool selected) override;
  void clear() override;
  [[nodiscard]] bool is_selected() const override;

private:
  PathIterator m_iterator;
  std::unique_ptr<TangentHandle> m_left_tangent_handle;
  std::unique_ptr<TangentHandle> m_right_tangent_handle;
  [[nodiscard]] bool tangents_active() const;

  void transform_tangent(const Vec2f& delta, TangentMode mode, TangentHandle::Tangent tangent);
};

}  // namespace omm
