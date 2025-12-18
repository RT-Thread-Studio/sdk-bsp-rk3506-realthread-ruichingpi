/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __ECAT_MASTER_H__
#define __ECAT_MASTER_H__

#include <stdint.h>

#define EC_MAX_STRING_LENGTH (64)
#define EC_MAX_PORTS         (4)

/** Master state.
 *
 * This is used for the output parameter of ecat_master_state().
 *
 * \see ecat_master_state().
 */
typedef struct
{
    unsigned int slaves_responding; /**< Sum of responding slaves on all
                                      Ethernet devices. */
    unsigned int al_states : 4;     /**< Application-layer states of all slaves.
                                      The states are coded in the lower 4 bits.
                                      If a bit is set, it means that at least one
                                      slave in the network is in the corresponding
                                      state:
                                      - Bit 0: \a INIT
                                      - Bit 1: \a PREOP
                                      - Bit 2: \a SAFEOP
                                      - Bit 3: \a OP */
    unsigned int link_up   : 1; /**< \a true, if at least one Ethernet link is
                                      up. */
} ec_master_state_t;

/****************************************************************************/
/** State of an EtherCAT slave.
 */
enum ec_slave_state
{
    EC_SLAVE_STATE_UNKNOWN = 0x00,
    /**< unknown state */
    EC_SLAVE_STATE_INIT = 0x01,
    /**< INIT state (no mailbox communication, no IO) */
    EC_SLAVE_STATE_PREOP = 0x02,
    /**< PREOP state (mailbox communication, no IO) */
    EC_SLAVE_STATE_BOOT = 0x03,
    /**< Bootstrap state (mailbox communication, firmware update) */
    EC_SLAVE_STATE_SAFEOP = 0x04,
    /**< SAFEOP (mailbox communication and input update) */
    EC_SLAVE_STATE_OP = 0x08,
    /**< OP (mailbox communication and input/output update) */
    EC_SLAVE_STATE_ACK_ERR = 0x10
    /**< Acknowledge/Error bit (no actual state) */
};

typedef struct
{
    uint8_t request_state; /**< request state of the slave. */
    uint8_t al_state;      /**< Current state of the slave. */
} ec_slave_state_t;
/****************************************************************************/

/** Slave information.
 *
 * This is used as an output parameter of ecrt_master_get_slave().
 *
 * \see ecrt_master_get_slave().
 */
typedef struct
{
    uint16_t index;           /**< Offset of the slave in the ring. */
    uint16_t state;           /** state of slave */
    uint16_t al_statuscode;   /** AL status code */
    uint16_t parent;          /** slave number for parent, 0=master */
    uint32_t vendor_id;       /**< Vendor-ID stored on the slave. */
    uint32_t product_code;    /**< Product-Code stored on the slave. */
    uint32_t revision_number; /**< Revision-Number stored on the slave. */
    uint32_t serial_number;   /**< Serial-Number stored on the slave. */
    uint16_t alias;           /**< The slaves alias if not equal to 0. */
    int16_t current_on_ebus;  /**< Used current in mA. */
    struct
    {
        uint32_t receive_time;     /**< Receive time on DC transmission delay
                                 measurement. */
        uint16_t next_slave;       /**< Ring position of next DC slave on that
                               port.  */
        uint32_t delay_to_next_dc; /**< Delay [ns] to next DC slave. */
    } ports[EC_MAX_PORTS];         /**< Port information. */
    uint8_t error_flag;            /**< Error flag for that slave. */
    uint8_t sync_count;            /**< Number of sync managers. */
    uint16_t sdo_count;            /**< Number of SDOs. */

    uint16_t mbx_proto;              /** mailbox supported protocols */
    uint16_t configadr;              /** Configured address */
    uint16_t Itype;                  /** Interface type */
    uint16_t Dtype;                  /** Device type */
    uint16_t Obits;                  /** output bits */
    uint16_t Ibits;                  /** input bits */
    uint8_t *outputs;
    uint8_t *inputs;
    int32_t pdelay;                  /** propagation delay */
    uint16_t DCnext;                 /** next DC slave */
    uint16_t DCprevious;             /** previous DC slave */
    int32_t DCcycle;                 /** DC cycle time in ns */
    int32_t DCshift;                 /** DC shift from clock modulus boundary */
    uint8_t DCactive;                /** DC sync activation, 0=off, 1=on */
    char name[EC_MAX_STRING_LENGTH]; /**< Name of the slave. */
} ec_slave_info_t;

/****************************************************************************/

/** Domain working counter interpretation.
 *
 * This is used in ec_domain_state_t.
 */
typedef enum
{
    EC_WC_ZERO = 0,   /**< No registered process data were exchanged. */
    EC_WC_INCOMPLETE, /**< Some of the registered process data were
                        exchanged. */
    EC_WC_COMPLETE    /**< All registered process data were exchanged. */
} ec_wc_state_t;

/****************************************************************************/

/** Domain state.
 *
 * This is used for the output parameter of ecat_domain_state().
 */
typedef struct
{
    unsigned int working_counter; /**< Value of the last working counter. */
    unsigned int
        expected_working_counter; /**< Value of the expected working counter. */
    ec_wc_state_t wc_state;       /**< Working counter interpretation. */
    unsigned int redundancy_active; /**< Redundant link is in use. */
} ec_domain_state_t;

/****************************************************************************/

/** Direction type for PDO assignment functions.
 */
typedef enum
{
    EC_DIR_OUTPUT,  /**< Values written by the master. */
    EC_DIR_INPUT,   /**< Values read by the master. */
} ec_direction_t;

/****************************************************************************/

/** Watchdog mode for sync manager configuration.
 *
 * Used to specify, if a sync manager's watchdog is to be enabled.
 */
typedef enum
{
    EC_WD_DEFAULT, /**< Use the default setting of the sync manager. */
    EC_WD_ENABLE,  /**< Enable the watchdog. */
    EC_WD_DISABLE, /**< Disable the watchdog. */
} ec_watchdog_mode_t;

/****************************************************************************/

/** PDO entry configuration information.
 *
 * This is the data type of the \a entries field in ec_pdo_info_t.
 *
 * \see ecrt_slave_config_pdos().
 */
typedef struct
{
    uint16_t index;     /**< PDO entry index. */
    uint8_t subindex;   /**< PDO entry subindex. */
    uint8_t bit_length; /**< Size of the PDO entry in bit. */
} ec_pdo_entry_info_t;

/****************************************************************************/

/** PDO configuration information.
 *
 * This is the data type of the \a pdos field in ec_sync_info_t.
 *
 * \see ecrt_slave_config_pdos().
 */
typedef struct
{
    uint16_t index;         /**< PDO index. */
    uint32_t n_entries; /**< Number of PDO entries in \a entries to map.
                                          Zero means, that the default mapping shall be
                                          used (this can only be done if the slave is
                                          present at configuration time). */
    ec_pdo_entry_info_t const *entries; /**< Array of PDO entries to map. Can
                                          either be \a NULL, or must contain
                                          at least \a n_entries values. */
} ec_pdo_info_t;

/****************************************************************************/

/** Sync manager configuration information.
 *
 * This can be use to configure multiple sync managers including the PDO
 * assignment and PDO mapping. It is used as an input parameter type in
 * ecrt_slave_config_pdos().
 */
typedef struct
{
    uint8_t    index;                /**< Sync manager index. */
    ec_direction_t dir;               /**< Sync manager direction. */
    uint32_t n_pdos;              /**< Number of PDOs in \a pdos. */
    ec_pdo_info_t const *pdos;        /**< Array with PDOs to assign. This must
                                        contain at least \a n_pdos PDOs. */
    ec_watchdog_mode_t watchdog_mode; /**< Watchdog mode. */
} ec_sync_info_t;

/****************************************************************************/

/** List record type for PDO entry mass-registration.
 *
 * This type is used for the array parameter of the
 * ecrt_domain_reg_pdo_entry_list()
 */
typedef struct
{
    uint16_t alias;        /**< Slave alias address. */
    uint16_t position;     /**< Slave position. */
    uint32_t vendor_id;    /**< Slave vendor ID. */
    uint32_t product_code; /**< Slave product code. */
    uint16_t index;        /**< PDO entry index. */
    uint8_t subindex;      /**< PDO entry subindex. */
    unsigned int *offset;  /**< Pointer to a variable to store the PDO entry's
                             (byte-)offset in the process data. */
    unsigned int *bit_position; /**< Pointer to a variable to store a bit
                                  position (0-7) within the \a offset. Can be
                                  NULL, in which case an error is raised if
                                  the PDO entry does not byte-align. */
} ec_pdo_entry_reg_t;

/****************************************************************************/

/** Request state.
 *
 * This is used as return type for ecrt_sdo_request_state() and
 * ecrt_voe_handler_state().
 */
typedef enum
{
    EC_REQUEST_UNUSED,  /**< Not requested. */
    EC_REQUEST_BUSY,    /**< Request is being processed. */
    EC_REQUEST_SUCCESS, /**< Request was processed successfully. */
    EC_REQUEST_ERROR,   /**< Request processing failed. */
} ec_request_state_t;

/****************************************************************************/

/** Application-layer state.
 */
typedef enum
{
    EC_AL_STATE_INIT = 0x01,   /**< Init. */
    EC_AL_STATE_PREOP = 0x02,  /**< Pre-operational. */
    EC_AL_STATE_SAFEOP = 0x04, /**< Safe-operational. */
    EC_AL_STATE_OP = 0x08,     /**< Operational. */
} ec_al_state_t;

typedef enum
{
    /** No valid state. */
    EC_STATE_NONE = 0x00,
    /** Init state*/
    EC_STATE_INIT = 0x01,
    /** Pre-operational. */
    EC_STATE_PRE_OP = 0x02,
    /** Boot state*/
    EC_STATE_BOOT = 0x03,
    /** Safe-operational. */
    EC_STATE_SAFE_OP = 0x04,
    /** Operational */
    EC_STATE_OPERATIONAL = 0x08,
    /** Error or ACK error */
    EC_STATE_ACK = 0x10,
    EC_STATE_ERROR = 0x10
} ec_state;

#define EC_EVENT_RUN  (0x01)
#define EC_EVENT_STOP (0x02)
#define EC_EVENT_LOOP (0x04)
#define EC_EVENT_EXIT (0x08)

typedef enum
{
    EC_NET_MODE_BRIDGING = 0x00,  /**< Bridging mode. */
    EC_NET_MODE_EXCLUSIVE = 0x01, /**< Exclusive mode. */
} ec_net_mode_t;

typedef struct ec_master
{
    const char *name;
    void *priv;
    const char *nic0;
    const char *nic1;
    const char *eni_file;
    uint32_t main_cycletime_us;
    uint32_t sub_cycletime_us;
    uint32_t recovery_timeout_ms;
    uint8_t *process_data;
    uint32_t process_data_size;
    uint32_t pdo_timeout;
    uint32_t sdo_tx_timeout;
    uint32_t sdo_rx_timeout;
    uint32_t info_cycle;
    uint32_t dc_cycltime0;
    uint32_t dc_cycltime1;
    int32_t dc_cyclshift;
    uint16_t dc_index;
    uint8_t dc_active;
    uint8_t dc_type;
    uint8_t pdi_check;
    uint8_t wdt_enable;
    uint32_t wdt_timeout;
    ec_master_state_t state;
    ec_domain_state_t domain_state;
    rt_thread_t domain_thread;
    struct rt_event event;
    uint8_t net_mode;
    uint8_t run;
    uint8_t priority;
    int64_t *dc_time;
    float pgain;
    float igain;
} ec_master_t;

struct _ecat_timer
{
    uint32_t sec;  /*< Seconds elapsed since the Epoch (Jan 1, 1970) */
    uint32_t usec; /*< Microseconds elapsed since last second boundary */
};

struct ecat_timer
{
    struct _ecat_timer stop_timer;
};

typedef void (*ec_pdo_callback_t)(uint16_t slave_index, uint8_t *output, uint8_t *input);

/** EtherCAT slave sync signal configuration.
 */
typedef struct {
    uint32_t cycle_time; /**< Cycle time [ns]. */
    int32_t shift_time;  /**< Shift time [ns]. */
} ec_sync_signal_t;

#define EC_SYNC_SIGNAL_COUNT (2)

typedef struct {
    ec_sync_info_t *sync;                           /**< Sync manager configuration. */
    uint8_t sync_count;                             /**< Number of sync managers. */
    ec_pdo_callback_t pdo_callback;                 /**< PDO process data callback. */
    uint16_t dc_assign_activate;                    /**< dc assign control */
    ec_sync_signal_t dc_sync[EC_SYNC_SIGNAL_COUNT]; /**< DC sync signals. */
} ec_slave_config_t;

rt_err_t ecat_master_init(ec_master_t *master);

rt_err_t ecat_master_deinit(ec_master_t *master);

rt_err_t ecat_master_start(ec_master_t *master);

rt_err_t ecat_simple_start(ec_master_t *master);

rt_err_t ecat_simple_stop(ec_master_t *master);

rt_err_t ecat_slave_config(
    ec_master_t *master, uint16_t slave, ec_slave_config_t *config);

rt_err_t ecat_master_state(ec_master_t *master, ec_master_state_t *state);

rt_err_t ecat_slave_state(
    ec_master_t *master, uint16_t slave, ec_slave_state_t *state);

rt_err_t ecat_slave_info(
    ec_master_t *master, uint16_t slave, ec_slave_info_t *info);

int ecat_sdo_write(ec_master_t *master,
    uint16_t slave,
    uint16_t index,
    uint8_t subindex,
    uint8_t complete_access,
    void *data,
    int size,
    int timeout);

int ecat_sdo_read(ec_master_t *master,
    uint16_t slave,
    uint16_t index,
    uint8_t subindex,
    uint8_t complete_access,
    void *data,
    int size,
    int timeout);

void ecat_dc_config(ec_master_t *master,
    uint16_t slave,
    uint8_t act,
    uint32_t cycle_time,
    int32_t cycle_shift);

void ecat_dc_config_ex(ec_master_t *master,
    uint16_t slave,
    uint8_t act,
    uint32_t cycle_time0,
    uint32_t cycle_time1,
    int32_t cycle_shift);

int ecat_slavecount(ec_master_t *master);

rt_inline int ecat_sdo_write_u8(ec_master_t *master,
    uint16_t slave,
    uint16_t index,
    uint8_t subindex,
    uint8_t data,
    int timeout)
{
    return ecat_sdo_write(master, slave, index, subindex, 0, &data, 1, timeout);
}

rt_inline int ecat_sdo_write_u16(ec_master_t *master,
    uint16_t slave,
    uint16_t index,
    uint8_t subindex,
    uint16_t data,
    int timeout)
{
    return ecat_sdo_write(master, slave, index, subindex, 0, &data, 2, timeout);
}

rt_inline int ecat_sdo_write_u32(ec_master_t *master,
    uint16_t slave,
    uint16_t index,
    uint8_t subindex,
    uint32_t data,
    int timeout)
{
    return ecat_sdo_write(master, slave, index, subindex, 0, &data, 4, timeout);
}

rt_inline int ecat_sdo_read_u8(ec_master_t *master,
    uint16_t slave,
    uint16_t index,
    uint8_t subindex,
    uint8_t *data,
    int timeout)
{
    return ecat_sdo_read(master, slave, index, subindex, 0, data, 1, timeout);
}

rt_inline int ecat_sdo_read_u16(ec_master_t *master,
    uint16_t slave,
    uint16_t index,
    uint8_t subindex,
    uint16_t *data,
    int timeout)
{
    return ecat_sdo_read(master, slave, index, subindex, 0, data, 2, timeout);
}

rt_inline int ecat_sdo_read_u32(ec_master_t *master,
    uint16_t slave,
    uint16_t index,
    uint8_t subindex,
    uint32_t *data,
    int timeout)
{
    return ecat_sdo_read(master, slave, index, subindex, 0, data, 4, timeout);
}

rt_err_t ecat_write_state(ec_master_t *master, uint16_t slave, uint16_t state);

rt_err_t ecat_check_state(
    ec_master_t *master, uint16_t slave, uint16_t *state, int timeout);

rt_err_t ecat_config_init(ec_master_t *master, uint8_t usetable);

rt_err_t ecat_config_dc(ec_master_t *master);

rt_err_t ecat_config_map_group(ec_master_t *master, void *map, uint8_t group);

rt_err_t ecat_send_processdata_group(ec_master_t *master, uint8_t group);

rt_err_t ecat_receive_processdata_group(
    ec_master_t *master, uint8_t group, int timeout);

rt_err_t ecat_timer_start(struct ecat_timer *t, uint32_t timeout_us);

rt_bool_t ecat_timer_is_expired(struct ecat_timer *t);

rt_err_t ecat_sync_dc(ec_master_t *master);

rt_err_t ecat_hwtimer_start(ec_master_t *master);

int ecat_service_init(void);

#endif /* __ECAT_MASTER_H__ */
