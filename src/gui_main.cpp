#include "gui/WorkbenchWindow.h"

#include <Windows.h>

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    return sec::WorkbenchWindow::run(instance, showCommand);
}
