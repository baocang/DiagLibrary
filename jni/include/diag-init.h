
#ifndef DIAG_INIT_H_
#define DIAG_INIT_H_

void is_DM_allowed(u_int32_t DMstate); /* Currently not used */
int  diag_com_write_command(int fd,u_int8_t deviceType,u_int16_t items[]);
int  diaginit_diag_com_write_command(u_int8_t deviceType,u_int16_t items[]);
int  diaginit_diag_com_write_event(u_int16_t Event_item[]);
int  diag_com_gps_write_command(int fd);
int  diag_com_gps_write_command(int fd);
int  Dm_cmd_time_request(int fd);
void diag_com_cdma_request(int fd,u_int16_t code);














/*
 * MDM VS. MSM
 * Reference: https://android.googlesource.com/kernel/msm.git/+/android-6.0.0_r0.9/include/linux/diagchar.h
 */
enum remote_procs {
	MSM = 0,
	MDM = 1,
	MDM2 = 2,
	QSC = 5,
};

/* Raw binary data type
 * Reference: https://android.googlesource.com/kernel/msm.git/+/android-6.0.0_r0.9/include/linux/diagchar.h
 */
#define MSG_MASKS_TYPE		0x00000001
#define LOG_MASKS_TYPE		0x00000002
#define EVENT_MASKS_TYPE	0x00000004
#define PKT_TYPE		0x00000008
#define DEINIT_TYPE		0x00000010
#define USER_SPACE_DATA_TYPE	0x00000020
#define DCI_DATA_TYPE		0x00000040
#define CALLBACK_DATA_TYPE	0x00000080
#define DCI_LOG_MASKS_TYPE	0x00000100
#define DCI_EVENT_MASKS_TYPE	0x00000200
#define DCI_PKT_TYPE		0x00000400

/* IOCTL commands for diagnostic port
 * Reference: https://android.googlesource.com/kernel/msm.git/+/android-6.0.0_r0.9/include/linux/diagchar.h
 */ 
#define DIAG_IOCTL_SWITCH_LOGGING	7
#define DIAG_IOCTL_LSM_DEINIT		9
#define DIAG_IOCTL_DCI_REG		23
#define DIAG_IOCTL_DCI_INIT		20
#define DIAG_IOCTL_DCI_DEINIT		21
#define DIAG_IOCTL_DCI_CLEAR_LOGS	28
#define DIAG_IOCTL_DCI_CLEAR_EVENTS	29
#define DIAG_IOCTL_REMOTE_DEV		32
#define DIAG_IOCTL_VOTE_REAL_TIME	33
#define DIAG_IOCTL_GET_REAL_TIME	34
#define DIAG_IOCTL_PERIPHERAL_BUF_CONFIG	35
#define DIAG_IOCTL_PERIPHERAL_BUF_DRAIN		36


#define MEMORY_DEVICE_MODE		2
#define CALLBACK_MODE		6
#define TTY_MODE			8

/* 
 * NEXUS-6-ONLY IOCTL
 * Reference: https://github.com/MotorolaMobilityLLC/kernel-msm/blob/kitkat-4.4.4-release-victara/include/linux/diagchar.h
 */
#define DIAG_IOCTL_OPTIMIZED_LOGGING	35
#define DIAG_IOCTL_OPTIMIZED_LOGGING_FLUSH	36


/* 
 * Buffering mode
 * Reference: https://android.googlesource.com/kernel/msm.git/+/android-6.0.0_r0.9/include/linux/diagchar.h
 */
#define DIAG_BUFFERING_MODE_STREAMING	0
#define DIAG_BUFFERING_MODE_THRESHOLD	1
#define DIAG_BUFFERING_MODE_CIRCULAR	2
#define DEFAULT_LOW_WM_VAL	15
#define DEFAULT_HIGH_WM_VAL	85
#define NUM_SMD_DATA_CHANNELS 4
#define NUM_SMD_CONTROL_CHANNELS NUM_SMD_DATA_CHANNELS

#define MODEM_DATA		0
#define LAST_PERIPHERAL 3


/* 
 * Structures for DCI client registration
 * Reference: https://android.googlesource.com/kernel/msm.git/+/android-6.0.0_r0.9/drivers/char/diag/diag_dci.h
 */
#define DCI_LOG_MASK_SIZE		(16*514)
#define DCI_EVENT_MASK_SIZE		512
struct diag_dci_reg_tbl_t {
	int client_id;
	uint16_t notification_list;
	int signal_type;
	int token;
} __packed;


/* 
 * Android 7.0: switch_logging_mode structure
 * Reference: https://android.googlesource.com/kernel/msm.git/+/android-7.1.0_r0.3/drivers/char/diag/diagchar.h
 */
struct diag_logging_mode_param_t {
	uint32_t req_mode;
	uint32_t peripheral_mask;
	uint8_t mode_param;
} __packed;
#define DIAG_CON_APSS		(0x0001)	/* Bit mask for APSS */
#define DIAG_CON_MPSS		(0x0002)	/* Bit mask for MPSS */
#define DIAG_CON_LPASS		(0x0004)	/* Bit mask for LPASS */
#define DIAG_CON_WCNSS		(0x0008)	/* Bit mask for WCNSS */
#define DIAG_CON_SENSORS	(0x0010)	/* Bit mask for Sensors */
#define DIAG_CON_NONE		(0x0000)	/* Bit mask for No SS*/
#define DIAG_CON_ALL		(DIAG_CON_APSS | DIAG_CON_MPSS \
				| DIAG_CON_LPASS | DIAG_CON_WCNSS \
				| DIAG_CON_SENSORS)

/* 
 * Structures for ioctl
 * Reference: https://android.googlesource.com/kernel/msm.git/+/android-6.0.0_r0.9/drivers/char/diag/diagchar_core.c
 */

typedef struct {
	char *p;
	size_t len;
} BinaryBuffer;

struct diag_buffering_mode_t {
	uint8_t peripheral;
	uint8_t mode;
	uint8_t high_wm_val;
	uint8_t low_wm_val;
} __packed;

#define DIAG_PROC_DCI			1
#define DIAG_PROC_MEMORY_DEVICE		2


struct real_time_vote_t {
	uint16_t proc;
	uint8_t real_time_vote;
};

struct real_time_query_t {
	int real_time;
	int proc;
} __packed;


/*
 * DCI structures
 */
struct diag_dci_client_tbl {
	struct task_struct *client;
	uint16_t list; /* bit mask */
	int signal_type;
	unsigned char dci_log_mask[DCI_LOG_MASK_SIZE];
	unsigned char dci_event_mask[DCI_EVENT_MASK_SIZE];
	unsigned char *dci_data;
	int data_len;
	int total_capacity;
	int dropped_logs;
	int dropped_events;
	int received_logs;
	int received_events;
};

/*
 * Default logging mode and buffer
 * Reference: https://android.googlesource.com/kernel/msm.git/+/android-6.0.0_r0.9/drivers/char/diag/diag_dci.h
 */

// char buf_read[BUFFER_SIZE] = {};	// From Haotian: improve reliability
// int mode = CALLBACK_MODE;	// Logging mode
static int mode = MEMORY_DEVICE_MODE;	// logging mode
static uint16_t remote_dev = 0; // MSM (0) or not
int client_id;	// DCI client ID (allocated by diag driver)
int fd; //file descriptor to /dev/diag


#endif /* DIAG_INIT_H_ */
