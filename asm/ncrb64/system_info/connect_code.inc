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

virtual at r8 
tp                   TOKEN_PRIVILEGES 
end virtual

;---------- Subroutines -------------------------------------------------------;

include 'load_inf.inc'
include 'load_kmd.inc'
include 'sysinfo.inc'

include 'cpuid_x64.inc'

include 'info_os.inc'
include 'info_native_os.inc'
include 'info_processor.inc'
include 'info_topology.inc'
include 'info_extended_topology.inc'
include 'info_numa_domains.inc'
include 'info_processor_groups.inc'
include 'info_acpi.inc'
include 'info_affinized_cpuid.inc'
