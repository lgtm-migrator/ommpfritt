#include "main/application.h"
#include "gtest/gtest.h"
#include "registers.h"
#include "testutil.h"
#include <QApplication>

class Environment : public ::testing::Environment
{
public:
  ~Environment() override = default;
  void SetUp() override
  {
    ommtest::qt_gui_app = std::make_unique<ommtest::GuiApplication>();
  }

  void TearDown() override
  {
    ommtest::qt_gui_app.reset();
  }
};

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(std::make_unique<Environment>().release());
  return RUN_ALL_TESTS();
}
