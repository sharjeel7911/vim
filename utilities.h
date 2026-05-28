// May 09, 2026
#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include <cctype> // for isdigit(), isalnum(), isspace()
#include <cstddef>
#include <cstring> // strlen, mmemcpy
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/ioctl.h>

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::right;
using std::setfill;
using std::setw;
using std::string;
using std::stringstream;
using std::to_string;
