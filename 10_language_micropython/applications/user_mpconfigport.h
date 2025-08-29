/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */

 /* options to control how user-defined modules is built */
#define MICROPY_PY_MY_LED           (1)

#if MICROPY_PY_MY_LED
extern const struct _mp_obj_module_t mp_module_my_led;
#define MICROPY_USER_MY_LED_MODULES \
    { MP_ROM_QSTR(MP_QSTR_my_led), MP_ROM_PTR(&mp_module_my_led) },
#endif

#ifndef MICROPY_USER_MY_LED_MODULES
#define MICROPY_USER_MY_LED_MODULES
#endif

/* extra user-defined modules names to add to the global namespace */
#define MICROPY_USER_MODULES \
    MICROPY_USER_MY_LED_MODULES
