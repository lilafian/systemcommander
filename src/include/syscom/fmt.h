/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares functions for formatting text.
 * Copyright (C) 2026 lilaf */

#pragma once

#include <stdarg.h>

void sprintf(char *str, const char *fmt, ...);
void vasprintf(char *str, const char *fmt, va_list args);
