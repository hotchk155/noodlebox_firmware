/*
 * GENERATED FILE - DO NOT EDIT
 * Copyright (c) 2008 - 2013 Code Red Technologies Ltd, 
 * Copyright 2015, 2018 NXP
 * (c) NXP Semiconductors 2013-2019
 * Generated linker script file for MKE04Z128xxx4
 * Created from memory.ldt by FMCreateLinkMemory
 * Using Freemarker v2.3.23
 * MCUXpresso IDE v10.3.1 [Build 2233] [2019-02-20] on 14-Oct-2019 17:56:57
 */

MEMORY
{
  /* Define each memory region */
  PROGRAM_FLASH (rx) : ORIGIN = 0x600, LENGTH = 0x1fa00 /* 129536 bytes (alias Flash) */  
  SRAM (rwx) : ORIGIN = 0x1ffff000, LENGTH = 0x4000 /* 16K bytes (alias RAM) */  
}

  /* Define a symbol for the top of each memory region */
  __base_PROGRAM_FLASH = 0x600  ; /* PROGRAM_FLASH */  
  __base_Flash = 0x600 ; /* Flash */  
  __top_PROGRAM_FLASH = 0x600 + 0x1fa00 ; /* 129536 bytes */  
  __top_Flash = 0x600 + 0x1fa00 ; /* 129536 bytes */  
  __base_SRAM = 0x1ffff000  ; /* SRAM */  
  __base_RAM = 0x1ffff000 ; /* RAM */  
  __top_SRAM = 0x1ffff000 + 0x4000 ; /* 16K bytes */  
  __top_RAM = 0x1ffff000 + 0x4000 ; /* 16K bytes */  
