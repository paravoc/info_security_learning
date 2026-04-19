#include "app/Application.h"

#include <windows.h>

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    sec::Application application;
    return application.run(argc, argv);
}
