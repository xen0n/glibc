/* Some compiler optimizations may transform loops into memset/memmove
   calls and without proper declaration it may generate PLT calls.  */
#if !defined __ASSEMBLER__ && IS_IN (libc) && defined SHARED
asm ("memmove = __GI_memmove");
asm ("memset = __GI_memset");
asm ("memcpy = __GI_memcpy");
#endif

/* -fstack-protector generates calls to __stack_chk_fail, which need
   similar adjustments to avoid going through the PLT.  */
#if !defined __ASSEMBLER__ && IS_IN (libc) && defined SHARED
asm ("__stack_chk_fail = __stack_chk_fail_local");
#endif
