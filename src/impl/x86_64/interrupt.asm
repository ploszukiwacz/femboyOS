[BITS 64]

; Export ISR handlers
global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31
global isr32
global isr33
global isr34
global isr35
global isr36
global isr37
global isr38
global isr39
global isr40
global isr41
global isr42
global isr43
global isr44
global isr45
global isr46
global isr47

; Reference to C handler function
extern isr_handler

; Common ISR stub that saves CPU state, calls C handler, and restores state
%macro ISR_NOERRCODE 1
isr%1:
    push 0                  ; Push dummy error code
    push %1                 ; Push interrupt number
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
isr%1:
    ; Error code already pushed by CPU
    push %1                 ; Push interrupt number
    jmp isr_common_stub
%endmacro

; Set up ISRs
ISR_NOERRCODE 0   ; Division by zero
ISR_NOERRCODE 1   ; Debug
ISR_NOERRCODE 2   ; Non-maskable interrupt
ISR_NOERRCODE 3   ; Breakpoint
ISR_NOERRCODE 4   ; Overflow
ISR_NOERRCODE 5   ; Bound range exceeded
ISR_NOERRCODE 6   ; Invalid opcode
ISR_NOERRCODE 7   ; Device not available
ISR_ERRCODE   8   ; Double fault
ISR_NOERRCODE 9   ; Coprocessor segment overrun
ISR_ERRCODE   10  ; Invalid TSS
ISR_ERRCODE   11  ; Segment not present
ISR_ERRCODE   12  ; Stack-segment fault
ISR_ERRCODE   13  ; General protection fault
ISR_ERRCODE   14  ; Page fault
ISR_NOERRCODE 15  ; Reserved
ISR_NOERRCODE 16  ; x87 floating-point exception
ISR_ERRCODE   17  ; Alignment check
ISR_NOERRCODE 18  ; Machine check
ISR_NOERRCODE 19  ; SIMD floating-point exception
ISR_NOERRCODE 20  ; Virtualization exception
ISR_NOERRCODE 21  ; Reserved
ISR_NOERRCODE 22  ; Reserved
ISR_NOERRCODE 23  ; Reserved
ISR_NOERRCODE 24  ; Reserved
ISR_NOERRCODE 25  ; Reserved
ISR_NOERRCODE 26  ; Reserved
ISR_NOERRCODE 27  ; Reserved
ISR_NOERRCODE 28  ; Reserved
ISR_NOERRCODE 29  ; Reserved
ISR_NOERRCODE 30  ; Reserved
ISR_NOERRCODE 31  ; Reserved

; IRQs
ISR_NOERRCODE 32  ; Timer
ISR_NOERRCODE 33  ; Keyboard
ISR_NOERRCODE 34  ; Cascade for PIC2
ISR_NOERRCODE 35  ; COM2
ISR_NOERRCODE 36  ; COM1
ISR_NOERRCODE 37  ; LPT2
ISR_NOERRCODE 38  ; Floppy Disk
ISR_NOERRCODE 39  ; LPT1
ISR_NOERRCODE 40  ; CMOS real-time clock
ISR_NOERRCODE 41  ; Free for peripherals
ISR_NOERRCODE 42  ; Free for peripherals
ISR_NOERRCODE 43  ; Free for peripherals
ISR_NOERRCODE 44  ; PS2 Mouse
ISR_NOERRCODE 45  ; FPU / Coprocessor
ISR_NOERRCODE 46  ; Primary ATA Hard Disk
ISR_NOERRCODE 47  ; Secondary ATA Hard Disk

; Common stub for handling interrupts
isr_common_stub:
    ; Save all registers
    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Call C handler
    mov rdi, [rsp + 15*8]  ; Pass interrupt number as first parameter
    call isr_handler

    ; Restore registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Clean up error code and interrupt number
    add rsp, 16
    iretq                   ; Return from interrupt
