#include <STC8H.H>

/* STC8 port uses UART interrupt reception instead of STM32 DMA1_Channel6. */
void MX_DMA_Init(void)
{
    /* Kept for source compatibility with the STM32 main.c call sequence. */
}
