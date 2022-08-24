;------------------------------------------------------------------------------;
;                                                                              ;
;     System Information Library: connect include files with subroutines.      ;
;                                                                              ;
;------------------------------------------------------------------------------;

;---------- Operating system definitions for large pages support --------------;

MEM_LARGE_PAGES          = 020000000h 
SE_PRIVILEGE_ENABLED     = 2 
SE_LOCK_MEMORY_PRIVILEGE = 4

struct LUID 
usedpart             dd ?   
ignorehigh32bitpart  dd ? 
ends 

struct LUID_AND_ATTRIBUTES 
Luid                 LUID 
Attributes           dd ?  
ends 

struct TOKEN_PRIVILEGES 
PrivilegeCount       dd ? 
Privileges           LUID_AND_ATTRIBUTES 
ends 

virtual at esi 
tp                   TOKEN_PRIVILEGES 
end virtual

;---------- Firmware definitions for SMBIOS support ---------------------------;

SMBIOS_TYPE_0       = 0
SMBIOS_UNKNOWN_TYPE = 45

struct DATA_HEADER
method       db 0
versionMajor db 0
versionMinor db 0
revisionDmi  db 0
dataLength   dd 0
ends

struct OBJECT_HEADER
type   db 0
length db 0
handle dw 0
ends

;---------- Equations for structures addressing -------------------------------;

OFFSET_P_CORES  equ  sizeof.SUMMARYCACHE + sizeof.SUMMARYTOPOLOGY  
OFFSET_E_CORES  equ  ( OFFSET_P_CORES ) * 2    

;---------- Subroutines -------------------------------------------------------;

include 'sysinfo.inc'
include 'cpuid_ia32.inc'
include 'info_os.inc'
include 'info_native_os.inc'
include 'info_processor.inc'
include 'info_topology.inc'
include 'info_extended_topology.inc'
include 'info_numa_domains.inc'
include 'info_processor_groups.inc'
include 'info_smbios.inc'
include 'info_acpi.inc'
include 'info_affinized_cpuid.inc'

