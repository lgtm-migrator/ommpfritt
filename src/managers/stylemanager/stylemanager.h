#pragma once

#include "managers/manager.h"
#include "managers/stylemanager/stylelistadapter.h"
#include "scene/scene.h"

namespace omm
{

class StyleManager : public Manager
{
  DECLARE_MANAGER_TYPE(StyleManager)

public:
  explicit StyleManager(Scene& scene);
  ~StyleManager();

  StyleListAdapter m_style_list_adapter;

};

}  // namespace omm
