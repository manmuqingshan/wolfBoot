/* spi_flash.h
 *
 * Generic implementation of the read/write/erase
 * functionalities, on top of the spi_drv.h HAL.
 *
 * Compile with SPI_FLASH=1
 *
 *
 * Copyright (C) 2022 wolfSSL Inc.
 *
 * This file is part of wolfBoot.
 *
 * wolfBoot is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * wolfBoot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

#ifndef SPI_FLASH_DRI_H
#define SPI_FLASH_DRI_H

/* SPI Configuration */
#ifndef SPI_FLASH_SECTOR_SIZE
#define SPI_FLASH_SECTOR_SIZE (4096)
#endif

#ifndef SPI_FLASH_PAGE_SIZE
#define SPI_FLASH_PAGE_SIZE   (256)
#endif

#if defined(SPI_FLASH) || defined(QSPI_FLASH) || defined(OCTOSPI_FLASH)

#include <stdint.h>

/* returns (manuf << 8 | product) */
uint16_t spi_flash_probe(void);
void spi_flash_release(void);

int spi_flash_sector_erase(uint32_t address);
int spi_flash_chip_erase(void);
int spi_flash_read(uint32_t address, void *data, int len);
int spi_flash_write(uint32_t address, const void *data, int len);

#else

#define spi_flash_probe() do{}while(0)
#define spi_flash_release() do{}while(0)

#endif /* SPI_FLASH || QSPI_FLASH || OCTOSPI_FLASH */

#endif /* !SPI_FLASH_DRI_H */
