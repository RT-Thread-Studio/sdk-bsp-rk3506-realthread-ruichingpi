/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */

#include "user_mpconfigport.h"

#if MICROPY_PY_MY_LED

#include "py/obj.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "modmachine.h"

#define LED1_PIN_NUM                (24)
#define LED2_PIN_NUM                (160)

typedef enum
{
    BOARD_LED_RED = 1,
    BOARD_LED_BLUE,
}board_led_t;

typedef struct _led_obj_t
{
    mp_obj_base_t base;
    rt_base_t pin;
    rt_bool_t active_level;
} led_obj_t;

STATIC void my_led_set_state(rt_base_t pin, int value)
{
    rt_pin_write(pin, value ? PIN_HIGH : PIN_LOW);
}

STATIC int my_led_get_state(rt_base_t pin)
{
    return rt_pin_read(pin);
}

STATIC mp_obj_t my_led_on(mp_obj_t self_in)
{
    led_obj_t *self = MP_OBJ_TO_PTR(self_in);
    my_led_set_state(self->pin, self->active_level);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(my_led_on_obj, my_led_on);

STATIC mp_obj_t my_led_off(mp_obj_t self_in)
{
    led_obj_t *self = MP_OBJ_TO_PTR(self_in);
    my_led_set_state(self->pin, !self->active_level);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(my_led_off_obj, my_led_off);

STATIC mp_obj_t my_led_toggle(mp_obj_t self_in)
{
    led_obj_t *self = MP_OBJ_TO_PTR(self_in);
    my_led_set_state(self->pin,
        my_led_get_state(self->pin)? PIN_LOW : PIN_HIGH);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(my_led_toggle_obj, my_led_toggle);

STATIC mp_obj_t my_led_make_new(const mp_obj_type_t *type, size_t n_args,
    size_t n_kw, const mp_obj_t *all_args)
{
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    led_obj_t *self = m_new_obj(led_obj_t);
    self->base.type = type;

    switch (mp_obj_get_int(all_args[0]))
    {
        case BOARD_LED_RED:
            self->pin = LED1_PIN_NUM;
            self->active_level = PIN_HIGH;
            break;
        case BOARD_LED_BLUE:
            self->pin = LED2_PIN_NUM;
            self->active_level = PIN_LOW;
            break;
        default:
            mp_raise_ValueError("Invalid LED, only support RED and BLUE");
    }

    rt_pin_mode(self->pin, PIN_MODE_OUTPUT);
    my_led_set_state(self->pin, !self->active_level);

    return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_rom_map_elem_t my_led_locals_dict_table[] =
{
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&my_led_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&my_led_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_toggle), MP_ROM_PTR(&my_led_toggle_obj) },

    { MP_ROM_QSTR(MP_QSTR_BLUE), MP_ROM_INT(BOARD_LED_BLUE) },
    { MP_ROM_QSTR(MP_QSTR_RED), MP_ROM_INT(BOARD_LED_RED) },
};
STATIC MP_DEFINE_CONST_DICT(my_led_locals_dict, my_led_locals_dict_table);

const mp_obj_type_t my_led_type =
{
    { &mp_type_type },
    .name = MP_QSTR_LED,
    .make_new = my_led_make_new,
    .locals_dict = (mp_obj_dict_t*)&my_led_locals_dict,
};

STATIC const mp_rom_map_elem_t my_led_module_globals_table[] =
{
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_my_led) },
    { MP_ROM_QSTR(MP_QSTR_LED), MP_ROM_PTR(&my_led_type) },
};
STATIC MP_DEFINE_CONST_DICT(my_led_module_globals, my_led_module_globals_table);

const mp_obj_module_t mp_module_my_led =
{
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&my_led_module_globals,
};

#endif // (MICROPY_PY_MY_LED && MICROPY_PY_PIN)
