// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// logger.h is a part of Blitzping.
// ---------------------------------------------------------------------

_Pragma ("once")
#ifndef LOGGER_H
#define LOGGER_H


#include <stdarg.h>
#include <stdbool.h>

#include <errno.h>
#include <stdio.h>
#include <time.h>

typedef enum LogLevel {
    LOG_NONE  = -1,
    LOG_CRIT  = 0,
    LOG_ERROR = 1,
    LOG_WARN  = 2,
    LOG_INFO  = 3,
    LOG_DEBUG = 4
} log_level_t;

void logger_set_level(const enum LogLevel level);
void logger_set_timestamps(const bool timestamp);
void logger(const enum LogLevel level, const char *const format, ...);

#define DEBUG_MSG(format, ...) (logger(LOG_DEBUG, \
    "[%s()@%s:%d] " format, __func__, __FILE__, __LINE__, __VA_ARGS__))

#endif // LOGGER_H

// ---------------------------------------------------------------------
// END OF FILE: logger.h
// ---------------------------------------------------------------------
