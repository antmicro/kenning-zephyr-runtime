#if defined(IREE_ARCH_ARM_32) || defined(IREE_ARCH_RISCV_32) || defined(IREE_ARCH_X86_32)
#define IREE_UK_POINTER_SIZE 4
#elif defined(IREE_ARCH_ARM_64) || defined(IREE_ARCH_RISCV_64) || defined(IREE_ARCH_X86_64)
#define IREE_UK_POINTER_SIZE 8
#endif
#ifdef IREE_ARCH_ARM_64
#define IREE_UK_ARCH_ARM_64
#endif // IREE_ARCH_ARM_64
