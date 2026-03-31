#pragma once

#include <stdarg.h>
#include <syscom/interrupts.h>

void panic(const char *msg);
void panicf(const char *fmt, ...);
void panic_interrupt(interrupt_frame *frame, const char *msg);
void panicf_interrupt(interrupt_frame *frame, const char *fmt, ...);
