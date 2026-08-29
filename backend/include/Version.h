#pragma once

#define PROJECT_VERSION "0.1.0"

#ifndef GIT_SHA
#define GIT_SHA "unknown"
#endif

constexpr const char VERSION[] = PROJECT_VERSION;
constexpr const char SOFTWARE_TYPE[] = "project-guardian/" PROJECT_VERSION "+" GIT_SHA;
