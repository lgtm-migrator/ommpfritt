#include "managers/stylemanager/stylemanager.h"

#include "commands/removecommand.h"
#include <QEvent>
#include "renderers/style.h"
#include "managers/stylemanager/stylelistview.h"
#include "scene/scene.h"
#include "commands/addcommand.h"
#include "mainwindow/application.h"
#include <QCoreApplication>
#include "scene/mailbox.h"
#include <QContextMenuEvent>

namespace omm
{


StyleManager::StyleManager(Scene& scene)
  : ItemManager( QCoreApplication::translate("any-context", "StyleManager"),
                 scene, scene.styles())
{
  connect(&scene.mail_box(), SIGNAL(selection_changed(std::set<Style*>)),
          &item_view(), SLOT(set_selection(std::set<Style*>)));
}

QString StyleManager::type() const { return TYPE; }

bool StyleManager::perform_action(const QString& action_name)
{
  if (action_name == "remove styles") {
    scene().remove(this, item_view().selected_items());
  } else {
    return false;
  }

  return true;
}

void StyleManager::contextMenuEvent(QContextMenuEvent* event)
{
  Application& app = Application::instance();
  KeyBindings& kb = app.key_bindings;
  const bool style_selected = !item_view().selected_items().empty();

  const auto e_os = [style_selected](QAction* action) {
    action->setEnabled(style_selected);
    return action;
  };
  QMenu menu(QCoreApplication::translate("any-context", StyleManager::TYPE));
  menu.addAction(e_os(kb.make_menu_action(*this, "remove styles").release()));
  menu.addAction(kb.make_menu_action(app, "new style").release());
  menu.addAction(kb.make_menu_action(app, "remove unused styles").release());

  menu.exec(event->globalPos());
}

}  // namespace omm
