;---------- Get Native OS information -----------------------------------------;
;                                                                              ;
; INPUT:   None                                                                ;
;                                                                              ;
; OUTPUT:  CF = Error flag, 0(NC) = OK, 1(C) = Error                           ;
;          AL = Error code, valid if CF = 1                                    ;
;                                                                              ;
;------------------------------------------------------------------------------;
InfoNativeOs:
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
mov rax,[DYNA_IMPORT._GetNativeSystemInfo]
test rax,rax
jz @f
lea rcx,[OS_DATA.nativeSystemInfo]
call rax
@@:
mov rsp,rbp
ret

;--- Build Native OS information for text report and GUI widgets --------------;
;                                                                              ;
; INPUT:   RBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  RBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
TextNativeOs:
mov [DYNA_PTR.textNativeOs.objectStart],rbp
mov bh,1                                  ; BH = 1 means native system info
call HelperSystemInfo
mov al,0
stosb
mov rbp,rdi
mov [DYNA_PTR.textNativeOs.objectStop],rbp
ret

