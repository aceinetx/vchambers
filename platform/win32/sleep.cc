#include <windows.h>
#include "sleep.hh"

void vc::Sleep(int milliseconds) {
    ::Sleep(milliseconds);
}
