
include (CheckCXXSourceCompiles)

CHECK_CXX_SOURCE_COMPILES(" int main() { __asm__(\"pxor %mm0, %mm0\") ; }" HAVE_X86_MMX)
CHECK_CXX_SOURCE_COMPILES(" int main() { __asm__(\"xorps %xmm0, %xmm0\"); }" HAVE_X86_SSE)
CHECK_CXX_SOURCE_COMPILES(" int main() { __asm__(\"xorpd %xmm0, %xmm0\"); }" HAVE_X86_SSE2)
CHECK_CXX_SOURCE_COMPILES(" int main() { __asm__(\"femms\"); }" HAVE_X86_3DNOW)
CHECK_CXX_SOURCE_COMPILES(" int main() { __asm__(\"mtspr 256, %0; vand %%v0, %%v0, %%v0\" : : \"r\"(-1) ); }" HAVE_PPC_ALTIVEC)
