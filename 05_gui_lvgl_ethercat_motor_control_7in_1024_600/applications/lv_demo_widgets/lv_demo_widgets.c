/**
 * @file lv_demo_widgets.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_demo_widgets.h"

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN && LV_MEM_SIZE < (38ul * 1024ul)
#error Insufficient memory for lv_demo_widgets. Please set LV_MEM_SIZE to at least 38KB (38ul * 1024ul).  48KB is recommended.
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
} disp_size_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void profile_create(lv_obj_t *parent);
static void slider_event_cb(lv_event_t *e);
static void delete_timer_event_cb(lv_event_t *e);
static void tabview_delete_event_cb(lv_event_t *e);

/**********************
 *  STATIC VARIABLES
 **********************/
static disp_size_t disp_size;

static lv_obj_t *tv;

static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_icon;
static lv_style_t style_bullet;

static lv_style_t font_style_18;
static lv_style_t font_style_22;

static lv_obj_t *scale;

static const lv_font_t *font_large;
static const lv_font_t *font_normal;

static uint8_t switch1_flag = 0;
static uint8_t switch2_flag = 1;

static int32_t motor_desired_pos;

extern int32_t motor_target_pos;
extern int32_t motor_current_pos;

extern int motor_run(void);
extern int motor_stop(void);
extern void motor_dir_set(uint8_t dir);

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_demo_widgets(void)
{
    if (LV_HOR_RES <= 320)
    {
        disp_size = DISP_SMALL;
    }
    else if (LV_HOR_RES < 720)
    {
        disp_size = DISP_MEDIUM;
    }
    else
    {
        disp_size = DISP_LARGE;
    }

    font_large = LV_FONT_DEFAULT;
    font_normal = LV_FONT_DEFAULT;

    int32_t tab_h;
    if (disp_size == DISP_LARGE)
    {
        tab_h = 70;
#if LV_FONT_MONTSERRAT_24
        font_large = &lv_font_montserrat_24;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_24 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
#if LV_FONT_MONTSERRAT_16
        font_normal = &lv_font_montserrat_16;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_16 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
    }

#if LV_USE_THEME_DEFAULT
    lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK,
                          font_normal);
#endif

    lv_style_init(&style_text_muted);
    lv_style_set_text_opa(&style_text_muted, LV_OPA_50);

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_large);

    lv_style_init(&style_icon);
    lv_style_set_text_color(&style_icon, lv_theme_get_color_primary(NULL));
    lv_style_set_text_font(&style_icon, font_large);

    lv_style_init(&font_style_18);
    lv_style_set_text_font(&font_style_18, &lv_font_montserrat_18);

    lv_style_init(&font_style_22);
    lv_style_set_text_font(&font_style_22, &lv_font_montserrat_22);

    lv_style_init(&style_bullet);
    lv_style_set_border_width(&style_bullet, 0);
    lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);

    tv = lv_tabview_create(lv_screen_active());
    lv_tabview_set_tab_bar_size(tv, tab_h);
    lv_obj_add_event_cb(tv, tabview_delete_event_cb, LV_EVENT_DELETE, NULL);

    lv_obj_set_style_text_font(lv_screen_active(), font_normal, 0);

    lv_obj_t *cont = lv_tabview_get_content(tv);
    lv_obj_t *page = lv_obj_create(cont);
    lv_obj_set_size(page, lv_pct(100), lv_pct(100));
    uint32_t tab_idx = lv_obj_get_child_count(cont);
    if (tab_idx == 1)
    {
        lv_tabview_set_active(tv, 0, LV_ANIM_OFF);
    }

    lv_obj_set_style_bg_color(page, lv_color_make(0x29, 0x3A, 0x4E), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_100, LV_PART_MAIN);

    lv_obj_t *t1 = page;

    if (disp_size == DISP_LARGE)
    {
        lv_obj_t *tab_bar = lv_tabview_get_tab_bar(tv);
        lv_obj_set_style_pad_left(tab_bar, LV_HOR_RES / 3, 0);

        lv_obj_set_style_bg_color(tab_bar, lv_color_hex(0x293A4E), 0);
        lv_obj_set_style_bg_opa(tab_bar, LV_OPA_100, LV_PART_MAIN);

        lv_obj_t *obj = lv_obj_create(tab_bar);
        lv_obj_set_flex_grow(obj, 1);
        lv_obj_set_size(obj, lv_pct(100), lv_pct(100));
        lv_group_t *g = lv_group_get_default();
        if (g)
        {
            lv_group_add_obj(g, obj);
        }

        lv_obj_t *label = lv_label_create(obj);
        lv_label_set_text(label, "EtherCat-Servo Motor Control");
        lv_obj_set_style_text_color(label, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN);
        lv_obj_add_style(label, &font_style_22, 0);

        lv_obj_t *logo = lv_image_create(tab_bar);
        lv_obj_add_flag(logo, LV_OBJ_FLAG_IGNORE_LAYOUT);
        LV_IMAGE_DECLARE(rtt_logo);
        lv_image_set_src(logo, &rtt_logo);

        lv_obj_align(logo, LV_ALIGN_LEFT_MID, -LV_HOR_RES / 3 + 75, 0);
    }

    profile_create(t1);
}

static void switch_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    char *btn_name = lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED)
    {
        if (0 == strcmp(btn_name, "Run/Stop"))
        {
            switch1_flag = !switch1_flag;
            if (switch1_flag)
            {
                motor_run();
            }
            else
            {
                motor_stop();
            }
        }
        else if (0 == strcmp(btn_name, "Direct/Reverse"))
        {
            switch2_flag = !switch2_flag;
            if (switch2_flag)
            {
                motor_dir_set(1);
            }
            else
            {
                motor_dir_set(0);
            }
        }
    }
}

static void des_pos_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    lv_obj_t *obj = timer->user_data;

    lv_label_set_text_fmt(obj, "%03ld", motor_desired_pos);
}

static void current_pos_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    lv_obj_t *obj = timer->user_data;

    lv_label_set_text_fmt(obj, "%03ld", motor_current_pos);
}

static void needle1_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    lv_obj_t *obj = timer->user_data;

    lv_scale_set_image_needle_value(scale, obj, motor_desired_pos);
}

static void needle2_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    lv_obj_t *obj = timer->user_data;

    lv_scale_set_image_needle_value(scale, obj, motor_current_pos);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void profile_create(lv_obj_t *parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW);

    /* Create the first panel */
    lv_obj_t *panel1 = lv_obj_create(parent);
    lv_obj_set_height(panel1, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(panel1, lv_color_hex(0x293A4E), 0);

    /* key - start/stop */
    char *run_stop_name = "Run/Stop";
    lv_obj_t *label1 = lv_label_create(panel1);
    lv_label_set_text(label1, run_stop_name);
    lv_obj_set_style_text_color(label1, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN);
    lv_obj_t *switch1 = lv_switch_create(panel1);
    lv_obj_set_width(switch1, 100);
    lv_obj_set_height(switch1, 50);
    lv_obj_add_event_cb(switch1, switch_event_cb, LV_EVENT_CLICKED, run_stop_name);
    lv_obj_set_style_bg_color(switch1, lv_color_hex(0x41F0F9), LV_STATE_CHECKED | LV_PART_INDICATOR);

    /* key - direct/reverse */
    char *direct_reverse_name = "Direct/Reverse";
    lv_obj_t *label2 = lv_label_create(panel1);
    lv_obj_set_style_text_color(label2, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN);
    lv_label_set_text(label2, direct_reverse_name);
    lv_obj_t *switch2 = lv_switch_create(panel1);
    lv_obj_set_width(switch2, 100);
    lv_obj_set_height(switch2, 50);
    lv_obj_add_event_cb(switch2, switch_event_cb, LV_EVENT_CLICKED, direct_reverse_name);

    /* Create the second panel */
    lv_obj_t *panel2 = lv_obj_create(parent);
    lv_obj_set_height(panel2, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(panel2, lv_color_hex(0x293A4E), 0);

    /* Expected position label */
    lv_obj_t *des_pos_label = lv_label_create(panel2);
    lv_label_set_text(des_pos_label, "Desired Position");
    lv_obj_set_style_text_color(des_pos_label, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN);
    lv_obj_add_style(des_pos_label, &style_title, 0);

    /* The value of the expected position */
    lv_obj_t *des_pos_val_label = lv_label_create(panel2);
    lv_label_set_text(des_pos_val_label, "000");
    lv_obj_set_style_text_color(des_pos_val_label, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN);
    lv_obj_add_style(des_pos_val_label, &font_style_18, 0);

    lv_timer_t *des_pos_timer = lv_timer_create(des_pos_timer_cb, 50, des_pos_val_label);
    lv_obj_add_event_cb(des_pos_val_label, delete_timer_event_cb, LV_EVENT_DELETE, des_pos_timer);

    /* Current location tag */
    lv_obj_t *current_pos_label = lv_label_create(panel2);
    lv_label_set_text(current_pos_label, "Current Position");
    lv_obj_set_style_text_color(current_pos_label, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN);
    lv_obj_add_style(current_pos_label, &style_title, 0);

    /* The value at the current position */
    lv_obj_t *current_pos_val_label = lv_label_create(panel2);
    lv_label_set_text(current_pos_val_label, "000");
    lv_obj_set_style_text_color(current_pos_val_label, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN);
    lv_obj_add_style(current_pos_val_label, &font_style_18, 0);

    lv_timer_t *current_pos_timer = lv_timer_create(current_pos_timer_cb, 50, current_pos_val_label);
    lv_obj_add_event_cb(current_pos_val_label, delete_timer_event_cb, LV_EVENT_DELETE, current_pos_timer);

    /* Create the third panel */
    lv_obj_t *panel3 = lv_obj_create(parent);
    lv_obj_set_height(panel3, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(panel3, lv_color_hex(0x293A4E), 0);

    /* Motor rotation label */
    lv_obj_t *motor_turn_label = lv_label_create(panel3);
    lv_label_set_text(motor_turn_label, "Motor turn");
    lv_obj_set_style_text_color(motor_turn_label, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN);
    lv_obj_add_style(motor_turn_label, &style_title, 0);

    /* Experience the motor label */
    lv_obj_t *experience_label = lv_label_create(panel3);
    lv_label_set_text(experience_label, "Experience");
    lv_obj_set_style_text_color(experience_label, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN);
    lv_obj_add_style(experience_label, &style_text_muted, 0);

    /* The motor rotation direction slider */
    lv_obj_t *slider1 = lv_slider_create(panel3);
    lv_obj_set_width(slider1, LV_PCT(95));
    lv_obj_set_height(slider1, 25);
    lv_slider_set_range(slider1, 0, 359);
    lv_obj_add_event_cb(slider1, slider_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(slider1, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
    lv_obj_refresh_ext_draw_size(slider1);

    lv_obj_set_style_bg_color(slider1, lv_color_hex(0x5AE2DC), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(slider1, LV_OPA_40, LV_PART_MAIN);

    /* Set the indicator (filled part) */
    lv_obj_set_style_bg_color(slider1, lv_color_hex(0x5AE2DC), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(slider1, LV_OPA_COVER, LV_PART_INDICATOR);

    /* Set the slider knob (drag button) */
    lv_obj_set_style_bg_color(slider1, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_obj_set_style_bg_opa(slider1, LV_OPA_COVER, LV_PART_KNOB);

    /* Create the fourth panel */
    lv_obj_t *panel4 = lv_obj_create(parent);
    lv_obj_set_height(panel4, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(panel4, 1);
    lv_obj_set_style_bg_color(panel4, lv_color_hex(0x293A4E), 0);

    lv_obj_t *title_label = lv_label_create(panel4);
    lv_label_set_text(title_label, "Desired-Current Position");
    lv_obj_set_style_text_color(title_label, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN);
    lv_obj_add_style(title_label, &style_title, 0);

    scale = lv_scale_create(panel4);
    lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_post_draw(scale, true);
    lv_obj_set_size(scale, 360, 360);

    static const char *scale_text[] =
    {
        "40", "50", "60", "70", "80", "90", "100", "110", "120", "130",
        "140", "150", "160", "170", "180", "190", "200", "210", "220", "230",
        "240", "250", "260", "270", "280", "290", "300", "310", "320", "330",
        "340", "350", "0", "10", "20", "30",
        NULL
    };
    lv_scale_set_range(scale, 0, 360);
    lv_scale_set_angle_range(scale, 360);
    lv_scale_set_text_src(scale, scale_text);
    lv_scale_set_total_tick_count(scale, 37);
    lv_obj_set_style_length(scale, 5, LV_PART_INDICATOR);
    lv_scale_set_major_tick_every(scale, 1);

    /* Set style - Main scale lines */
    lv_obj_set_style_arc_color(scale, lv_color_hex(0x5AE2DC), LV_PART_MAIN);
    lv_obj_set_style_line_color(scale, lv_color_hex(0x5AE2DC), LV_PART_MAIN);

    /* Set Style - Indicator Section (if necessary) */
    lv_obj_set_style_arc_color(scale, lv_color_hex(0x5AE2DC), LV_PART_INDICATOR);
    lv_obj_set_style_line_color(scale, lv_color_hex(0x5AE2DC), LV_PART_INDICATOR);

    /* Set the text color (optional) */
    lv_obj_set_style_text_color(scale, lv_color_hex(0x5AE2DC), LV_PART_MAIN);

    /* Create a pointer */
    LV_IMG_DECLARE(point_orange);
    LV_IMG_DECLARE(point_blue);

    lv_obj_t *needle1 = lv_image_create(scale);
    lv_image_set_src(needle1, &point_orange);
    lv_image_set_pivot(needle1, 67, 97);
    lv_obj_align(needle1, LV_ALIGN_CENTER, 33, -35);

    lv_timer_t *needle1_timer = lv_timer_create(needle1_timer_cb, 50, needle1);
    lv_obj_add_event_cb(needle1, delete_timer_event_cb, LV_EVENT_DELETE, needle1_timer);

    lv_obj_t *needle2 = lv_image_create(scale);
    lv_image_set_src(needle2, &point_blue);
    lv_image_set_pivot(needle2, 67, 97);
    lv_obj_align(needle2, LV_ALIGN_CENTER, 33, -35);

    lv_timer_t *needle2_timer = lv_timer_create(needle2_timer_cb, 50, needle2);
    lv_obj_add_event_cb(needle2, delete_timer_event_cb, LV_EVENT_DELETE, needle2_timer);

    if (disp_size == DISP_LARGE)
    {
        /* The columns and rows on the main panel */
        static int32_t grid_main_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        static int32_t grid_main_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

        /* The columns and rows of the first panel */
        static int32_t grid_1_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        static int32_t grid_1_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

        /* The columns and rows of the second pane */
        static int32_t grid_2_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        static int32_t grid_2_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

        /* The columns and rows of the third panel */
        static int32_t grid_3_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        static int32_t grid_3_row_dsc[] =
        {
            LV_GRID_CONTENT,
            LV_GRID_CONTENT,
            30,
            LV_GRID_CONTENT,
            LV_GRID_TEMPLATE_LAST
        };

        /* The columns and rows on the fourth panel */
        static int32_t grid_4_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        static int32_t grid_4_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

        lv_obj_set_grid_dsc_array(parent, grid_main_col_dsc, grid_main_row_dsc);
        lv_obj_set_grid_cell(parent, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 0, 1);

        lv_obj_set_grid_dsc_array(panel1, grid_1_col_dsc, grid_1_row_dsc);
        lv_obj_set_grid_cell(panel1, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
        lv_obj_set_grid_cell(label1, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
        lv_obj_set_grid_cell(label2, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
        lv_obj_set_grid_cell(switch1, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
        lv_obj_set_grid_cell(switch2, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

        lv_obj_set_grid_dsc_array(panel2, grid_2_col_dsc, grid_2_row_dsc);
        lv_obj_set_grid_cell(panel2, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
        lv_obj_set_grid_cell(des_pos_label, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
        lv_obj_set_grid_cell(des_pos_val_label, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
        lv_obj_set_grid_cell(current_pos_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
        lv_obj_set_grid_cell(current_pos_val_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

        lv_obj_set_grid_dsc_array(panel3, grid_3_col_dsc, grid_3_row_dsc);
        lv_obj_set_grid_cell(panel3, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
        lv_obj_set_grid_cell(motor_turn_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
        lv_obj_set_grid_cell(experience_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
        lv_obj_set_grid_cell(slider1, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 3, 1);

        lv_obj_set_grid_dsc_array(panel4, grid_4_col_dsc, grid_4_row_dsc);
        lv_obj_set_grid_cell(panel4, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 3);
        lv_obj_set_grid_cell(title_label, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
        lv_obj_set_grid_cell(scale, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 1, 1);
    }

    lv_obj_update_layout(parent);
}

static void slider_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_REFR_EXT_DRAW_SIZE)
    {
        int32_t *s = lv_event_get_param(e);
        *s = LV_MAX(*s, 60);
    }
    else if (code == LV_EVENT_DRAW_TASK_ADDED)
    {
        lv_draw_task_t *draw_task = lv_event_get_param(e);
        if (draw_task == NULL || draw_task->type != LV_DRAW_TASK_TYPE_FILL)
        {
            return;
        }
        lv_draw_rect_dsc_t *draw_rect_dsc = draw_task->draw_dsc;

        if (draw_rect_dsc->base.part == LV_PART_KNOB && lv_obj_has_state(obj, LV_STATE_PRESSED))
        {
            char buf[8];
            lv_snprintf(buf, sizeof(buf), "%" LV_PRId32, lv_slider_get_value(obj));

            motor_desired_pos = atoi(buf);

            lv_point_t text_size;
            lv_text_get_size(&text_size, buf, font_normal, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            lv_area_t txt_area;
            txt_area.x1 = draw_task->area.x1 + lv_area_get_width(&draw_task->area) / 2 - text_size.x / 2;
            txt_area.x2 = txt_area.x1 + text_size.x;
            txt_area.y2 = draw_task->area.y1 - 10;
            txt_area.y1 = txt_area.y2 - text_size.y;

            lv_area_t bg_area;
            bg_area.x1 = txt_area.x1 - LV_DPX(8);
            bg_area.x2 = txt_area.x2 + LV_DPX(8);
            bg_area.y1 = txt_area.y1 - LV_DPX(8);
            bg_area.y2 = txt_area.y2 + LV_DPX(8);

            lv_draw_rect_dsc_t rect_dsc;
            lv_draw_rect_dsc_init(&rect_dsc);
            rect_dsc.bg_color = lv_palette_darken(LV_PALETTE_GREY, 3);
            rect_dsc.radius = LV_DPX(5);
            lv_draw_rect(draw_rect_dsc->base.layer, &rect_dsc, &bg_area);

            lv_draw_label_dsc_t label_dsc;
            lv_draw_label_dsc_init(&label_dsc);
            label_dsc.color = lv_color_white();
            label_dsc.font = font_normal;
            label_dsc.text = buf;
            label_dsc.text_local = 1;
            lv_draw_label(draw_rect_dsc->base.layer, &label_dsc, &txt_area);
        }

        if (lv_obj_get_state(obj) == LV_STATE_FOCUSED)
        {
            char buf[8];
            lv_snprintf(buf, sizeof(buf), "%" LV_PRId32, lv_slider_get_value(obj));

            motor_target_pos = atoi(buf);
        }
    }
}

static void delete_timer_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_DELETE)
    {
        lv_timer_t *t = lv_event_get_user_data(e);
        if (t)
        {
            lv_timer_delete(t);
        }
    }
}

static void tabview_delete_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_DELETE)
    {
        lv_style_reset(&style_text_muted);
        lv_style_reset(&style_title);
        lv_style_reset(&style_icon);
        lv_style_reset(&style_bullet);
    }
}

void lv_user_gui_init(void)
{
    lv_demo_widgets();
}
