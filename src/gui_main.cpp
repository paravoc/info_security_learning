#include "gui/WorkbenchWindow.h"

#include <windows.h>

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    return sec::WorkbenchWindow::run(instance, showCommand);
}
