; i386 fast forwarder thunk implementations
; Calling convention: https://docs.microsoft.com/en-us/cpp/cpp/stdcall

.MODEL FLAT

    extrn ___guard_check_icall_fptr:DWORD

.CODE

InvokeForwarder PROC

    ; Replace forwarder abi with owner abi 
    mov     ecx, dword ptr[esp + 4]
    mov     edx, dword ptr[ecx + 4]
    mov     [esp + 4], edx
    
    ; Add offset and index
    add     eax, dword ptr [ecx + 8]  

    ; Get method address from owner abi vtable 
    mov     ecx, dword ptr [edx]  
    mov     eax, dword ptr [ecx + eax * 4]  

    ; Verify indirect call target
    call [___guard_check_icall_fptr]

    ; Jump to method
    jmp     eax         

InvokeForwarder ENDP

; Define thunks
FAST_FWD_THUNK MACRO i
_fast_fwd_thunk&i&@0 PROC 
    mov eax, i
    jmp InvokeForwarder 
_fast_fwd_thunk&i&@0 ENDP 
PUBLIC _fast_fwd_thunk&i&@0
ENDM

include thunks.inc

END