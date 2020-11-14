#pragma once

#include "external/json_fwd.hpp"
#include <QDialog>
#include <memory>

namespace Ui
{
class ToolBarDialog;
}

class QIdentityProxyModel;

namespace omm
{
class KeyBindings;
class KeyBindingsProxyModel;
class ToolBarItemModel;

class ToolBarDialog : public QDialog
{
  Q_OBJECT
public:
  ToolBarDialog(ToolBarItemModel& model, QWidget* parent = nullptr);
  ~ToolBarDialog();
  static constexpr auto mime_type = "application/command";

private:
  KeyBindings& m_key_bindings;
  std::unique_ptr<Ui::ToolBarDialog> m_ui;
  std::unique_ptr<KeyBindingsProxyModel> m_proxy;
  ToolBarItemModel& m_model;
};

}  // namespace omm
