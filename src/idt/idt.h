#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct interrupt_frame
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t unused_esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t ip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed));

/* 
 * idt_zero - handler for interrupt 0
 */
void idt_zero();

/*
 * idt_set - set the ith entry in the idt with the given handler function
 *
 * i - the interrupt number
 * handler - address of the interrupt/exception handler 
 */ 
void idt_set(int i, void *handler);

void idt_init();

void enable_interrupts();

void disable_interrupts();

/* Interrupt handler for int 0x80.  The kernel calls `int 0x80` when it wants to invoke
 * a kernel command.  The specific command from userland is passed via eax and is represented by the command parameter here.
 * The interrupt_frame stores the registers that were passed to the kernel on the stack when
 * the user program issued the `int` instruction (that instruction puts them on the stack for us).
 */
void *isr80h_handler(int command, struct interrupt_frame *frame);

#endif /* IDT_H */
