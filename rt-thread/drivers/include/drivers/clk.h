/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __CLK_H__
#define __CLK_H__

#include <rtthread.h>
#include <drivers/ofw.h>
#include <kpi.h>

#define RT_CLK_NAME_MAX 32

#define CLK_IS_ROOT     (1 << 0)

struct rt_clk;

struct rt_clk_ops
{
    rt_int32_t (*prepare)(struct rt_clk *clk);
    void (*unprepare)(struct rt_clk *clk);
    rt_err_t (*enable)(struct rt_clk *clk);
    rt_err_t (*disable)(struct rt_clk *clk);
    rt_bool_t (*is_enabled)(struct rt_clk *clk);
    rt_uint64_t (*recalc_rate)(struct rt_clk *clk, rt_uint64_t parent_rate);
    rt_uint64_t (*round_rate)(
        struct rt_clk *clk, rt_uint64_t parent_rate, rt_uint64_t rate);
    rt_err_t (*set_parent)(struct rt_clk *clk, rt_uint8_t index);
    rt_uint8_t (*get_parent)(struct rt_clk *clk);
    rt_err_t (*set_rate)(
        struct rt_clk *clk, rt_ubase_t parent_rate, rt_ubase_t rate);
    rt_uint64_t (*get_rate)(struct rt_clk *clk, rt_uint64_t parent_rate);
    rt_int32_t (*set_rate_and_parent)(struct rt_clk *clk,
        rt_uint64_t rate,
        rt_uint64_t parent_rate,
        rt_uint8_t index);
    rt_uint64_t (*recalc_accuracy)(
        struct rt_clk *clk, rt_uint64_t parent_accuracy);
    rt_int32_t (*init)(struct rt_clk *clk);
};

struct rt_clk
{
    char name[RT_CLK_NAME_MAX];
    rt_uint32_t count;
    rt_uint32_t id;
    struct rt_ofw_node *np;
    struct rt_clk_ops *ops;
    struct rt_clk **parents;
    rt_uint8_t parent_num;
    rt_list_t children;
    rt_list_t child_node;
    rt_uint32_t flags;
    rt_list_t node;
    void *pdata;
};

struct rt_clk_array
{
    rt_uint32_t count;
    struct rt_clk *clks[];
};

char *rt_strchrnul(const char *s, int c);
size_t rt_strcspn(const char *s, const char *c);
char *rt_strsep(char **str, const char *sep);

typedef struct rt_clk *(*__kpi_rt_clk_find)(
    struct rt_ofw_node *np, const char *name);
typedef struct rt_clk *(*__kpi_rt_clk_find_by_id)(rt_uint32_t id);
typedef struct rt_clk *(*__kpi_rt_clk_find_by_index)(
    struct rt_ofw_node *np, rt_uint32_t index);
typedef struct rt_clk_array *(*__kpi_rt_clk_find_array)(struct rt_ofw_node *np);
typedef struct rt_clk *(*__kpi_rt_clk_create)(rt_uint32_t id,
    const char *name,
    struct rt_clk_ops *ops,
    rt_uint8_t parent_num,
    rt_uint32_t pdata_len,
    rt_uint32_t flags);
typedef void (*__kpi_rt_clk_delete)(struct rt_clk *clk);
typedef rt_err_t (*__kpi_rt_clk_register)(
    struct rt_ofw_node *np, struct rt_clk *clk);
typedef rt_err_t (*__kpi_rt_clk_unregister)(struct rt_clk *clk);

typedef const char *(*__kpi_rt_clk_get_name)(struct rt_clk *clk);
typedef rt_err_t (*__kpi_rt_clk_set_parent)(
    struct rt_clk *clk, struct rt_clk *parent);
typedef struct rt_clk *(*__kpi_rt_clk_get_parent)(struct rt_clk *clk);
typedef void (*__kpi_rt_clk_enable)(struct rt_clk *clk);
typedef void (*__kpi_rt_clk_disable)(struct rt_clk *clk);
typedef rt_bool_t (*__kpi_rt_clk_get_status)(struct rt_clk *clk);
typedef rt_uint64_t (*__kpi_rt_clk_set_rate)(
    struct rt_clk *clk, rt_uint64_t rate);
typedef rt_uint64_t (*__kpi_rt_clk_get_rate)(struct rt_clk *clk);
typedef rt_uint64_t (*__kpi_rt_clk_round_rate)(
    struct rt_clk *clk, rt_uint64_t rate);

/* new */
typedef rt_err_t (*__kpi_rt_clk_prepare)(struct rt_clk *clk);
typedef rt_err_t (*__kpi_rt_clk_unprepare)(struct rt_clk *clk);
typedef rt_err_t (*__kpi_rt_clk_prepare_enable)(struct rt_clk *clk);
typedef void (*__kpi_rt_clk_disable_unprepare)(struct rt_clk *clk);
typedef rt_err_t (*__kpi_rt_clk_array_prepare)(struct rt_clk_array *clk_arr);
typedef rt_err_t (*__kpi_rt_clk_array_unprepare)(struct rt_clk_array *clk_arr);
typedef rt_err_t (*__kpi_rt_clk_array_enable)(struct rt_clk_array *clk_arr);
typedef void (*__kpi_rt_clk_array_disable)(struct rt_clk_array *clk_arr);
typedef rt_err_t (*__kpi_rt_clk_array_prepare_enable)(
    struct rt_clk_array *clk_arr);
typedef void (*__kpi_rt_clk_array_disable_unprepare)(
    struct rt_clk_array *clk_arr);
typedef void (*__kpi_rt_clk_put)(struct rt_clk *clk);
typedef struct rt_clk *(*__kpi_rt_ofw_get_clk_by_name)(
    struct rt_ofw_node *np, const char *name);

/* clk-fixed-rate.c */
typedef struct rt_clk *(*__kpi_rt_clk_fixed_rate_register)(
    struct rt_ofw_node *np, const char *name, rt_uint64_t rate);
typedef rt_err_t (*__kpi_rt_clk_fixed_rate_unregister)(struct rt_clk *clk);

/* clk-fixed-factor.c */
typedef struct rt_clk *(*__kpi_rt_clk_fixed_factor_register)(
    struct rt_ofw_node *np,
    rt_uint32_t id,
    const char *name,
    const char *parent,
    rt_uint32_t mult,
    rt_uint32_t div);
typedef rt_err_t (*__kpi_rt_clk_fixed_factor_unregister)(struct rt_clk *clk);

typedef int (*__kpi_of_clk_add_provider)(struct rt_ofw_node *np,
    struct rt_clk *(*clk_src_get)(struct rt_ofw_cell_args *clkspec, void *data),
    void *data);
typedef struct rt_clk *(*__kpi_rt_clk_get_by_name)(
    struct rt_device *dev, const char *name);

KPI_EXTERN(rt_clk_find);
KPI_EXTERN(rt_clk_find_by_id);
KPI_EXTERN(rt_clk_find_by_index);
KPI_EXTERN(rt_clk_find_array);
KPI_EXTERN(rt_clk_create);
KPI_EXTERN(rt_clk_delete);
KPI_EXTERN(rt_clk_register);
KPI_EXTERN(rt_clk_unregister);
KPI_EXTERN(rt_clk_get_name);
KPI_EXTERN(rt_clk_set_parent);
KPI_EXTERN(rt_clk_get_parent);
KPI_EXTERN(rt_clk_enable);
KPI_EXTERN(rt_clk_disable);
KPI_EXTERN(rt_clk_get_status);
KPI_EXTERN(rt_clk_set_rate);
KPI_EXTERN(rt_clk_get_rate);
KPI_EXTERN(rt_clk_round_rate);
KPI_EXTERN(rt_clk_prepare);
KPI_EXTERN(rt_clk_unprepare);
KPI_EXTERN(rt_clk_prepare_enable);
KPI_EXTERN(rt_clk_disable_unprepare);
KPI_EXTERN(rt_clk_array_prepare);
KPI_EXTERN(rt_clk_array_unprepare);
KPI_EXTERN(rt_clk_array_enable);
KPI_EXTERN(rt_clk_array_disable);
KPI_EXTERN(rt_clk_array_prepare_enable);
KPI_EXTERN(rt_clk_array_disable_unprepare);
KPI_EXTERN(rt_clk_put);
KPI_EXTERN(rt_ofw_get_clk_by_name);
KPI_EXTERN(rt_clk_fixed_rate_register);
KPI_EXTERN(rt_clk_fixed_rate_unregister);
KPI_EXTERN(rt_clk_fixed_factor_register);
KPI_EXTERN(rt_clk_fixed_factor_unregister);
KPI_EXTERN(of_clk_add_provider);
KPI_EXTERN(rt_clk_get_by_name);

#endif /* __CLK_H__ */
