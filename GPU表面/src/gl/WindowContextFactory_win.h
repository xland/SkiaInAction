
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_win_DEFINED
#define WindowContextFactory_win_DEFINED

#include <Windows.h>

#include <memory>

namespace skwindow {

class WindowContext;
struct DisplayParams;

std::unique_ptr<WindowContext> MakeGLForWin(HWND, const DisplayParams&);


std::unique_ptr<WindowContext> MakeRasterForWin(HWND, const DisplayParams&);

}  // namespace skwindow

#endif
