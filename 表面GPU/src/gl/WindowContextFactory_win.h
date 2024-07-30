#pragma once

#include <Windows.h>
#include <memory>

namespace skwindow {

class WindowContext;

std::unique_ptr<WindowContext> MakeGLForWin(HWND);

}
