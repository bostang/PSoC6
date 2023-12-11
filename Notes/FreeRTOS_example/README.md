# FreeRTOS Example

## Tahapan

1. Tambahkan:
```
FreeRTOS
clib-support
abstraction-rtos
```

ke library manager.

2. Tambahkan ke Makefile:
```
COMPONENTS+=FREERTOS
```
3. Salin `FreeRTOSConfig.h` dari `freertos/Source/portable/COMPONENT_$(CORE)` ke folder proyek (sejajar dengan main.c / paling luar)
4. Buka FreeRTOSConfig.h, hapus baris :
```
#warning This is a template. line.
```

