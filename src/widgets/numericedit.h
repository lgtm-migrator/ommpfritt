#pragma once

#include <QLineEdit>
#include <sstream>
#include <numeric>
#include <cmath>
#include <QWheelEvent>
#include <memory>
#include <sstream>
#include <iomanip>
#include <QCoreApplication>
#include "logging.h"
#include "properties/numericproperty.h"

namespace omm
{

class AbstractNumericEdit : public QLineEdit
{
  Q_OBJECT
public:
  using QLineEdit::QLineEdit;

Q_SIGNALS:
  void value_changed();
};

template<typename ValueType>
class NumericEdit : public AbstractNumericEdit
{
public:
  const QString inf = "inf";
  const QString neg_inf = "-inf";

  using value_type = ValueType;
  NumericEdit(QWidget* parent = nullptr) : AbstractNumericEdit(parent)
  {
    setContextMenuPolicy(Qt::NoContextMenu);

    connect(this, &QLineEdit::textEdited, [this](const QString& text) {
      const auto value = parse(text);
      if (value != m_last_value) {
        m_value = value;
        m_last_value = m_value;
        Q_EMIT value_changed();
      }
    });

    connect(this, &QLineEdit::editingFinished, [this]() { set_value(value()); });

    set_text(invalid_value);
  }

  void set_range(const value_type min, const value_type max)
  {
    if (min > max) {
      LERROR << "min is greater than max: " << min << " > " << max;
      m_min = min;
      m_max = min;
    } else {
      m_min = min;
      m_max = max;
    }
  }

  void set_lower(const value_type min)
  {
    if (min > m_max) {
      LERROR << "min is greater than max: " << min << " > " << m_max;
    } else {
      m_min = min;
    }
  }

  void set_upper(const value_type max)
  {
    if (m_min > max) {
      LERROR << "max is greater than min: " << m_min << " > " << max;
    } else {
      m_max = max;
    }
  }

  void set_step(value_type step)
  {
    assert(step > 0);
    m_step = step;
  }

  void set_multiplier(double multiplier) { m_multiplier = multiplier; }

  void set_value(value_type value)
  {
    value = std::clamp(value, m_min, m_max);
    if (std::isnan(value)) {
      set_invalid_value();
    } else {
      if (value != this->value() || !hasFocus()) {
        set_text(value);
        m_value = value;
        if (hasFocus()) { Q_EMIT value_changed(); }
      }
    }
  }

  void set_invalid_value() { setText(QObject::tr("< invalid >", "property")); }
  value_type value() const
  {
    return std::clamp(m_value, m_min, m_max);
  }

protected:
  void wheelEvent(QWheelEvent* e) override
  {
    // TODO
    // e->pixelDelta() is always null for me.
    // e->angleDelta() is (0, +-120) with my logitech mouse
    // e->angleDelta() is (0, n*8) with trackpoint (n being integer)
    // to make behaviour of trackpoint and mouse somewhat consistent, don't use the value
    // of angleDelta but only its direction.
    // curiously, `xev` does not display values on trackpoint/mouse wheel
    //  scroll but only button press events.

    // const auto dy = e->angleDelta().y();
    // if (dy < 0) {
    //   e->accept();
    //   increment(-1); }
    // else if (dy > 0) {
    //   e->accept();
    //   increment(1);
    // }

    QLineEdit::wheelEvent(e);
  }

  void mousePressEvent(QMouseEvent* e) override
  {
    if (e->button() == Qt::RightButton) {
      m_mouse_press_pos = e->pos();
      e->accept();
    } else {
      QLineEdit::mousePressEvent(e);
    }
  }

  void mouseMoveEvent(QMouseEvent* e) override
  {
    if (e->buttons() & Qt::RightButton) {
      QPoint distance = m_mouse_press_pos - e->pos();
      increment(distance.y());
      QCursor::setPos(mapToGlobal(m_mouse_press_pos));
      e->accept();
    } else {
      QLineEdit::mouseMoveEvent(e);
    }
  }

  void keyPressEvent(QKeyEvent* e) override
  {
    if (e->key() == Qt::Key_Down) {
      increment(-1);
      e->accept();
    } else if (e->key() == Qt::Key_Up) {
      increment(1);
      e->accept();
    } else {
      QLineEdit::keyPressEvent(e);
    }
  }

private:
  value_type m_min = NumericProperty<value_type>::lowest_possible_value;
  value_type m_max = NumericProperty<value_type>::highest_possible_value;
  value_type m_step = 1;
  double m_multiplier = 1.0;
  QPoint m_mouse_press_pos;

  void increment(double factor)
  {
    const auto increment = factor * m_step / m_multiplier;

    // do the range checking in double-domain.
    double new_value = double(this->value()) + double(increment);
    if (new_value > m_max) {
      new_value = m_max;
    } else if (new_value < m_min) {
      new_value = m_min;
    }
    set_value(value_type(new_value));
  }

  value_type parse(const QString& text) const
  {
    if (text == inf) {
      return NumericProperty<value_type>::highest_possible_value;
    } else if (text == neg_inf) {
      return NumericProperty<value_type>::lowest_possible_value;
    } else {
      std::istringstream sstream(text.toStdString());
      value_type value;
      sstream >> value;
      value /= m_multiplier;
      if (sstream) {
        return value;
      } else {
        if (m_min <= 0 && 0 <= m_max) {
          return 0;
        } else {
          return m_min;
        }
      }
    }
  }

  void set_text(const value_type& value)
  {
    m_value = value;
    std::ostringstream ss;
    ss << std::setprecision(3) << std::fixed << value_type(m_multiplier * value);
    const auto new_text = QString::fromStdString(ss.str());
    if (text() != new_text) {
      setText(new_text);
    }
    m_last_value = value;
  }
  static constexpr value_type invalid_value = 0;

public:
  static auto make_range_edits()
  {
    auto min_edit = std::make_unique<NumericEdit<ValueType>>();
    auto& min_edit_ref = *min_edit;
    auto max_edit = std::make_unique<NumericEdit<ValueType>>();
    auto& max_edit_ref = *max_edit;
    connect(min_edit.get(), &AbstractNumericEdit::value_changed, [&min_edit_ref, &max_edit_ref]() {
      const auto high = NumericProperty<value_type>::highest_possible_value;
      max_edit_ref.set_range(min_edit_ref.value(), high);
    });
    connect(max_edit.get(), &AbstractNumericEdit::value_changed, [&min_edit_ref, &max_edit_ref]() {
      const auto low = NumericProperty<value_type>::lowest_possible_value;
      min_edit_ref.set_range(low, min_edit_ref.value());
    });
    return std::pair(std::move(min_edit), std::move(max_edit));
  }

private:
  ValueType m_last_value;
  ValueType m_value;
};

using IntNumericEdit = NumericEdit<int>;
using DoubleNumericEdit = NumericEdit<double>;

}  // namespace
