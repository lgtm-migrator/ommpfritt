#pragma once

#include "preferences/preferencestree.h"
#include "color/color.h"
#include <QPalette>

namespace omm
{

class UiColors : public PreferencesTree
{
  Q_OBJECT
public:
  explicit UiColors();
  ~UiColors();
  QVariant data(int column, const PreferencesTreeValueItem &item, int role) const override;
  bool set_data(int column, PreferencesTreeValueItem& item, const QVariant &value) override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  Color color(QPalette::ColorGroup pgroup, const QString& group, const QString& name) const;
  void apply() override;

  QStringList skin_names() const;
  void set_skin(std::size_t index);
  std::size_t skin_index() const { return m_current_skin_index; }

  static void draw_background(QPainter& painter, const QRectF& rect);

protected:
  QString translation_context() const override { return "uicolors"; }

private:
  struct SkinInfo
  {
    const QString display_name;
    const QString file_name;
    const QString qsettings_group;
  };
  std::vector<SkinInfo> m_skins;
  int m_current_skin_index = -1;
  QPalette make_palette() const;
};

QColor ui_color(const QWidget& widget, const QString& group, const QString& name);
QColor ui_color(const QPalette::ColorGroup& status, const QString& group, const QString& name);
QColor ui_color(const QWidget& widget, const QPalette::ColorRole& role);
QColor ui_color(const QString& group, const QString& name);

}  // namespace omm
