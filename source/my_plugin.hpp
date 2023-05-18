// make sure header is read only once
#ifndef MY_PLUGIN_HPP
#define MY_PLUGIN_HPP
#include <reascript_vararg.hpp>
#include <cstdio>
#include <string>
#include <locale>
#include <codecvt>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <vector>
#include <math.h>
#include <limits>
#include <Windows.h>
#include <stdio.h>
#include <direct.h>
#include <map>
#include <iostream>
#include <algorithm>


void RegisterMyPlugin();
void UnregisterMyPlugin();
const int MAIN_SECTION = 0;
const int MIDI_EDITOR_SECTION = 32060;
const int MIDI_EVENT_LIST_SECTION = 32061;
const int MEDIA_EXPLORER_SECTION = 32063;


#endif