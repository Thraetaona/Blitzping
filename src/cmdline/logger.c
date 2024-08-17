// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// logger.c is a part of Blitzping.
// ---------------------------------------------------------------------


#include "logger.h"


static enum LogLevel CURRENT_LOG_LEVEL = LOG_DEBUG;
static bool LOG_TIMESTAMPS = true;

// TODO: Mutexes? (for thread-safety)
void logger_set_level(const enum LogLevel level) {
    CURRENT_LOG_LEVEL = level;
}

void logger_set_timestamps(const bool timestamp) {
    LOG_TIMESTAMPS = timestamp;
}


#define TIMESTAMP_SIZE 26
_Static_assert(TIMESTAMP_SIZE >= 20,
    "timestamp array must be at least 20 characters long");

// NOTE: size should be at least 20.
static int generate_timestamp(
    char *const timestamp, const size_t size
) {
    const time_t now = time(NULL);
    if (now == ((time_t) -1)) {
        snprintf(timestamp, size, "%-*s",
            (int)(size-1), "unknown time");
        return 1;
    }

    const struct tm *const t = localtime(&now);
    if (t == NULL) {
        snprintf(
            timestamp, size,
            "U-%ld%-*s", (long)now, (int)(size - 1
            - snprintf(NULL, 0, "U-%ld", (long)now)), ""
        );
        return 1;
    }

    if (strftime(timestamp, size, "%Y/%m/%d@%H:%M:%S", t) == 0) {
        snprintf(timestamp, size,
            "%04d/%02d/%02d@%02d:%02d:%02d",
            (t->tm_year + 1900) % 10000,
            (t->tm_mon + 1) % 13,
            t->tm_mday % 32,
            t->tm_hour % 24,
            t->tm_min % 60,
            t->tm_sec % 60
        );
        return 1;
    }

    return 0;
}

// NOTE: These are ordered (according to the enum LogLevel).
static const char *const LEVEL_STRINGS[] = {
    "CRIT",
    "ERRR",
    "WARN",
    "INFO",
    "DBUG"
};

void logger(const enum LogLevel level, const char *const format, ...) {
    if (level > CURRENT_LOG_LEVEL) {
        return;
    }
    const int old_errno = errno; // Save the old errno value

    FILE *const output_stream = level <= LOG_WARN ? stderr : stdout;

    if (LOG_TIMESTAMPS) {
        char timestamp[TIMESTAMP_SIZE] = {0};
        // Get the current time (if possible)
        (void)generate_timestamp(timestamp, TIMESTAMP_SIZE);
        fprintf(output_stream, "[%s|%s] ",
            LEVEL_STRINGS[level], timestamp);
    }
    else {
        fprintf(output_stream, "[%s] ",
            LEVEL_STRINGS[level]);
    }

    va_list args;
    va_start(args, format);
    errno = 0;
    const int print_status = vfprintf(output_stream, format, args);
    va_end(args);

    fprintf(output_stream, "\n");

    // TODO: See if there is a better way to error-check these.
    if (ferror(output_stream) || print_status < 0) {
        perror("Error logging message");
        clearerr(output_stream);
    }

    errno = 0; // Reset errno
    if (fflush(output_stream) == EOF) {
        perror("Error flushing stream");
    }

    // Check for errors on the output stream
    if (ferror(output_stream)) {
        perror("Error after flushing stream");
        clearerr(output_stream);
    }

    errno = old_errno; // Restore the old errno value
}


// ---------------------------------------------------------------------
// END OF FILE: logger.c
// ---------------------------------------------------------------------
