#include <syscom/interrupts.h>
#include <syscom/panic.h>
#include <syscom/string.h>

void register_interrupt_handler(idtr idtr,uint8_t vectornum,uint64_t handler_offset){
    idt_descriptor_entry* interrupt_page_fault = (idt_descriptor_entry*)(idtr.offset+vectornum*sizeof(idt_descriptor_entry));
    idtdesce_set_offset(interrupt_page_fault,handler_offset);
    interrupt_page_fault->type_attributes = IDT_TYPE_ATTRIB_INTERRUPT_GATE;
    interrupt_page_fault->selector = 0x08;
}

__attribute__((interrupt)) void inthdlr_page_fault(interrupt_frame* frame,uint64_t error_code){
    panicf_interrupt(frame,"Page fault\nerror_code = 0b%s\n",itoa(error_code,2));
}

__attribute__((interrupt)) void inthdlr_division_error(interrupt_frame* frame){
    panic_interrupt(frame,"Division error\n");
}

__attribute__((interrupt)) void inthdlr_bound_range_exceeded(interrupt_frame* frame){
    panic_interrupt(frame,"Bound range exceeded\n");
}

__attribute__((interrupt)) void inthdlr_invalid_opcode(interrupt_frame* frame){
    panic_interrupt(frame,"Invalid opcode\n");
}

__attribute__((interrupt)) void inthdlr_device_not_available(interrupt_frame* frame){
    panic_interrupt(frame,"Device not available\n");
}

__attribute__((interrupt)) void inthdlr_double_fault(interrupt_frame* frame,uint64_t error_code){
    panicf_interrupt(frame,"Double fault\nerror_code = 0b%s\n",itoa(error_code,2));
}

__attribute__((interrupt)) void inthdlr_invalid_tss(interrupt_frame* frame,uint64_t error_code){
    panicf_interrupt(frame,"Invalid TSS\nerror_code = 0b%s\n",itoa(error_code,2));
}

__attribute__((interrupt)) void inthdlr_segment_not_present(interrupt_frame* frame,uint64_t error_code){
    panicf_interrupt(frame,"Segment not present\nerror_code = 0b%s\n",itoa(error_code,2));
}

__attribute__((interrupt)) void inthdlr_stackseg_fault(interrupt_frame* frame,uint64_t error_code){
    panicf_interrupt(frame,"Stack segment fault\nerror_code = 0b%s\n",itoa(error_code,2));
}

__attribute__((interrupt)) void inthdlr_genprotect_fault(interrupt_frame* frame,uint64_t error_code){
    panicf_interrupt(frame,"General protection fault\nerror_code = 0b%s\n",itoa(error_code,2));
}

__attribute__((interrupt)) void inthdlr_fp_exception(interrupt_frame* frame){
    panic_interrupt(frame,"Floating point exception\n");
}

__attribute__((interrupt)) void inthdlr_alignment_check(interrupt_frame* frame,uint64_t error_code){
    panicf_interrupt(frame,"Alignment check\nerror_code = 0b%s\n",itoa(error_code,2));
}

__attribute__((interrupt)) void inthdlr_simd_fp_exception(interrupt_frame* frame){
    panic_interrupt(frame,"SIMD floating point exception\n");
}

__attribute__((interrupt)) void inthdlr_virtualization_exception(interrupt_frame* frame){
    panic_interrupt(frame,"Virtualization exception\n");
}

__attribute__((interrupt)) void inthdlr_conprotect_exception(interrupt_frame* frame,uint64_t error_code){
    panicf_interrupt(frame,"Control protection exception\nerror_code = 0b%s\n",itoa(error_code,2));
}

__attribute__((interrupt)) void inthdlr_hypervisor_inj_exception(interrupt_frame* frame){
    panic_interrupt(frame,"Hypervisor injection exception\n");
}

__attribute__((interrupt)) void inthdlr_vmm_communication_exception(interrupt_frame* frame,uint64_t error_code){
    panicf_interrupt(frame,"VMM communication exception\nerror_code = 0b%s\n",itoa(error_code,2));
}

__attribute__((interrupt)) void inthdlr_security_exception(interrupt_frame* frame,uint64_t error_code){
    panicf_interrupt(frame,"Security exception\nerror_code = 0b%s\n",itoa(error_code,2));
}
