#pragma once

#include <stdint.h>

#define IDT_TYPE_ATTRIB_INTERRUPT_GATE 0b10001110
#define IDT_TYPE_ATTRIB_CALL_GATE 0b10001100
#define IDT_TYPE_ATTRIB_TRAP_GATE 0b10001111

typedef struct {
    uint16_t offset0;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attributes;
    uint16_t offset1;
    uint32_t offset2;
    uint32_t _ignore;
}__attribute__((packed)) idt_descriptor_entry;

void idtdesce_set_offset(idt_descriptor_entry* entry, uint64_t offset);
uint64_t idtdesce_get_offset(idt_descriptor_entry* entry);

typedef struct {
    uint16_t limit;
    uint64_t offset;
}__attribute__((packed)) idtr;
