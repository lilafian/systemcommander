#pragma once
#include <stdint.h>
#include <syscom/idt.h>

typedef struct interrupt_frame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} interrupt_frame;

void register_interrupt_handler(idtr idtr_struct,uint8_t vectornum,uint64_t handler_offset);
__attribute__((interrupt)) void inthdlr_page_fault(interrupt_frame* frame,uint64_t error_code);
__attribute__((interrupt)) void inthdlr_division_error(interrupt_frame* frame);
__attribute__((interrupt)) void inthdlr_bound_range_exceeded(interrupt_frame* frame);
__attribute__((interrupt)) void inthdlr_invalid_opcode(interrupt_frame* frame);
__attribute__((interrupt)) void inthdlr_device_not_available(interrupt_frame* frame);
__attribute__((interrupt)) void inthdlr_double_fault(interrupt_frame* frame,uint64_t error_code);
__attribute__((interrupt)) void inthdlr_invalid_tss(interrupt_frame* frame,uint64_t error_code);
__attribute__((interrupt)) void inthdlr_segment_not_present(interrupt_frame* frame,uint64_t error_code);
__attribute__((interrupt)) void inthdlr_stackseg_fault(interrupt_frame* frame,uint64_t error_code);
__attribute__((interrupt)) void inthdlr_genprotect_fault(interrupt_frame* frame,uint64_t error_code);
__attribute__((interrupt)) void inthdlr_fp_exception(interrupt_frame* frame);
__attribute__((interrupt)) void inthdlr_alignment_check(interrupt_frame* frame,uint64_t error_code);
__attribute__((interrupt)) void inthdlr_simd_fp_exception(interrupt_frame* frame);
__attribute__((interrupt)) void inthdlr_virtualization_exception(interrupt_frame* frame);
__attribute__((interrupt)) void inthdlr_conprotect_exception(interrupt_frame* frame,uint64_t error_code);
__attribute__((interrupt)) void inthdlr_hypervisor_inj_exception(interrupt_frame* frame);
__attribute__((interrupt)) void inthdlr_vmm_communication_exception(interrupt_frame* frame,uint64_t error_code);
__attribute__((interrupt)) void inthdlr_security_exception(interrupt_frame* frame,uint64_t error_code);
