#include "app.h"

#include "support.h"
#include "binding_helpers.h"
#include "bindings.h"
#include "resources.h"
#include "resource_manager.h"
#include "scenario.h"
#include "scenario_bsp.h"
#include "scenario_physics.h"

App::App() {
  _messageLogger = make_shared<MessageLogger>();
  MessagesFromWeb::getInstance()->registerHandler(_messageLogger);

  // _currentScenario = make_shared<TestScenario>();
  // _currentScenario = make_shared<PopTartScenario>();
  // _currentScenario = make_shared<BSPScenario>();
  _currentScenario = make_shared<PhysicsScenario>();
}

void App::loop(GLFWwindow* window) {
  static bool error = false;
  if (hasErrors() || error) {
    error = true;
    return;
  }

  static bool ready = false;
  if (!ready) {
    LoadingState loadingState = ResourceManager::getInstance()->think();
    switch (loadingState) {
      case LoadingState::FAILED:
        error = true;
        return;
      case LoadingState::DONE:
        ready = true;
        return;
      default:
        return;
    }
  }

  glm::vec2 dir = {0, 0};
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { dir += glm::vec2(0.0, 1.0); }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { dir += glm::vec2(1.0, 0.0); }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { dir += glm::vec2(-1.0, 0.0); }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { dir += glm::vec2(0.0, -1.0); }

  if (glm::length(dir) > 0) {
    dir = glm::normalize(dir);
  }

  static optional<double> oldX, oldY;
  static double pitch = - 3.14159 * 0.5;
  static double yaw = 0;

  double x, y;
  glfwGetCursorPos(window, &x, &y);

  if (oldX && oldY) {
    double dx = x - *oldX;
    double dy = y - *oldY;
    pitch += dy / 400.0f;
    yaw +=  dx / 400.0f;
    if(pitch > 89.0) pitch =  89.0;
    if(pitch < -89.0) pitch = -89.0;
  }
  oldX = x;
  oldY = y;

  _currentScenario->think(dir, pitch, yaw);
  _currentScenario->render();

  glfwSwapBuffers(window);
  glfwPollEvents();
}
