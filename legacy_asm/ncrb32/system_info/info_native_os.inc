;---------- Get Native OS information -----------------------------------------;
;                                                                              ;
; INPUT:   None                                                                ;
;                                                                              ;
; OUTPUT:  CF = Error flag, 0(NC) = OK, 1(C) = Error                           ;
;          AL = Error code, valid if CF = 1                                    ;
;                                                                              ;
;------------------------------------------------------------------------------;
InfoNativeOs:
mov eax,[DYNA_IMPORT._GetNativeSystemInfo]
test eax,eax
jz @f
lea ecx,[OS_DATA.nativeSystemInfo]
push ecx
call eax
@@:
ret

;--- Build Native OS information for text report and GUI widgets --------------;
;                                                                              ;
; INPUT:   EBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  EBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
TextNativeOs:
mov [DYNA_PTR.textNativeOs.objectStart],ebp
mov bh,1                                  ; BH = 1 means native system info
call HelperSystemInfo
mov al,0
stosb
mov ebp,edi
mov [DYNA_PTR.textNativeOs.objectStop],ebp
ret

