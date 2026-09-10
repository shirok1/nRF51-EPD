/* nRF51822 xxAB (128 KiB flash / 16 KiB RAM), S110 8.0.0.
 * Reserve the final two 1 KiB pages for pstorage data and swap.
 */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x18000, LENGTH = 0x7800
  RAM (rwx) : ORIGIN = 0x20002000, LENGTH = 0x2000
}
INCLUDE "nrf5x_common.ld"
/* The SDK places the RAM initializer at a flash load address explicitly. */
ASSERT(__etext + SIZEOF(.data) <= ORIGIN(FLASH) + LENGTH(FLASH),
       "application overlaps pstorage flash pages")
