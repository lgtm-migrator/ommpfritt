#pragma once

#include "tools/handles/moveaxishandle.h"
#include "tools/handles/particlehandle.h"
#include "tools/handles/rotatehandle.h"
#include "tools/handles/scaleaxishandle.h"
#include "tools/handles/scalebandhandle.h"
#include "objects/path.h"
#include "tools/selecttool.h"
#include <memory>

namespace omm
{

class TransformPointsHelper;

class SelectPointsBaseTool : public AbstractSelectTool
{
public:
  explicit SelectPointsBaseTool(Scene& scene);
  ~SelectPointsBaseTool() override;
  static constexpr auto TANGENT_MODE_PROPERTY_KEY = "tangent_mode";
  static constexpr auto BOUNDING_BOX_MODE_PROPERTY_KEY = "bounding_box_mode";
  enum class BoundingBoxMode { IncludeTangents, ExcludeTangents, None };
  PointSelectHandle::TangentMode tangent_mode() const;
  std::unique_ptr<QMenu> make_context_menu(QWidget* parent) override;
  void transform_objects(ObjectTransformation t) override;

  bool mouse_press(const Vec2f& pos, const QMouseEvent& event) override;
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event, bool allow_clear);

  bool has_transformation() const override;

  template<typename ToolT> static void make_handles(ToolT& tool, bool force_subhandles = false)
  {
    tool.handles.push_back(std::make_unique<ScaleBandHandle<ToolT>>(tool));
    tool.handles.push_back(std::make_unique<RotateHandle<ToolT>>(tool));

    static constexpr auto X = AxisHandleDirection::X;
    static constexpr auto Y = AxisHandleDirection::Y;
    tool.handles.push_back(std::make_unique<MoveAxisHandle<ToolT, X>>(tool));
    tool.handles.push_back(std::make_unique<MoveAxisHandle<ToolT, Y>>(tool));
    tool.handles.push_back(std::make_unique<ScaleAxisHandle<ToolT, X>>(tool));
    tool.handles.push_back(std::make_unique<ScaleAxisHandle<ToolT, Y>>(tool));

    tool.handles.push_back(std::make_unique<MoveParticleHandle<ToolT>>(tool));

    for (auto* path : tool.scene()->template item_selection<Path>()) {
      tool.handles.reserve(tool.handles.size() + path->point_count());
      for (auto* point : path->points()) {
        auto handle = std::make_unique<PointSelectHandle>(tool, *path, *point);
        handle->force_draw_subhandles = force_subhandles;
        tool.handles.push_back(std::move(handle));
      }
    }
  }

  BoundingBox bounding_box() const;
  void transform_objects_absolute(ObjectTransformation t);
  void on_property_value_changed(Property* property) override;
  SceneMode scene_mode() const override
  {
    return SceneMode::Vertex;
  }

protected:
  Vec2f selection_center() const override;

private:
  std::unique_ptr<TransformPointsHelper> m_transform_points_helper;
};

class SelectPointsTool : public SelectPointsBaseTool
{
public:
  using SelectPointsBaseTool::SelectPointsBaseTool;
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SelectPointsTool");

protected:
  void reset() override;
};

}  // namespace omm
