#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <errno.h>
#include <endian.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include "errors.h"
#include "interfaceapi.h"
#include "diag-cmd.h"
#include "diag-init.h"
#include "ipcQueue.h"

extern int gManufactureType;
extern int globalFD;
extern int init_done;

#define LOGD printf

void *diag_com_listen(void *arg);
extern void exit();


/*function to open device*/
 int  diag_com_open ()
{
	int status;
	int fd;
	status = fd = open("/dev/diag_mdm", O_RDWR , O_NONBLOCK ,O_NDELAY);/* open Lte device*/
	if (status == -1) {
		status = fd = open("/dev/diag",O_RDWR);/*open Wcdma device*/
		if (status == -1) {
			d_warning("diag_com_ioctl : Not any device is open");
		} else {
			printf("\nWcdma/Gsm device is open");
		}
	} else {
		printf("\nLte Device is open");
	}
	return fd;
}

/*function to config ioctl on  device*/
int diag_com_ioctl(int fd)
{
	
	int ret;


	/*
     * EXPERIMENTAL (NEXUS 6 ONLY): 
     * 1. check remote_dev
     * 2. Register a DCI client
     * 3. Send DCI control command
     */
    ret = ioctl(fd, DIAG_IOCTL_REMOTE_DEV, (char *) &remote_dev); 
    if (ret < 0){
	        printf("ioctl DIAG_IOCTL_REMOTE_DEV fails, with ret val = %d\n", ret);
	    	perror("ioctl DIAG_IOCTL_REMOTE_DEV");
	} 
	else{
		LOGD("DIAG_IOCTL_REMOTE_DEV remote_dev=%d\n",remote_dev);
	}

	// Register a DCI client
	struct diag_dci_reg_tbl_t dci_client;
	dci_client.client_id = 0;
	dci_client.notification_list = 0;
	dci_client.signal_type = SIGPIPE;
	// dci_client.token = remote_dev;
	dci_client.token = 0;
	ret = ioctl(fd, DIAG_IOCTL_DCI_REG, (char *) &dci_client); 
    if (ret < 0){
	        printf("ioctl DIAG_IOCTL_DCI_REG fails, with ret val = %d\n", ret);
	    	perror("ioctl DIAG_IOCTL_DCI_REG");
	} 
	else{
		client_id = ret;
		printf("DIAG_IOCTL_DCI_REG client_id=%d\n", client_id);
	}

	// Nexus-6-only logging optimizations
	unsigned int b_optimize = 1;
	ret = ioctl(fd, DIAG_IOCTL_OPTIMIZED_LOGGING, (char *) &b_optimize); 
	if (ret < 0){
	        printf("ioctl DIAG_IOCTL_OPTIMIZED_LOGGING fails, with ret val = %d\n", ret);
	    	perror("ioctl DIAG_IOCTL_OPTIMIZED_LOGGING");
	} 
	// ret = ioctl(fd, DIAG_IOCTL_OPTIMIZED_LOGGING_FLUSH, NULL); 
	// if (ret < 0){
	//         printf("ioctl DIAG_IOCTL_OPTIMIZED_LOGGING_FLUSH fails, with ret val = %d\n", ret);
	//     	perror("ioctl DIAG_IOCTL_OPTIMIZED_LOGGING_FLUSH");
	// } 


	/*
     * TODO: cleanup the diag before start
     * 1. Drain the buffer: prevent outdate logs next time
     * 2. Clean up masks: prevent enable_log bug next time
     */

	/*
	 * DIAG_IOCTL_LSM_DEINIT, try if it can clear buffer
	 */

	/*
	ret = ioctl(fd, DIAG_IOCTL_LSM_DEINIT, NULL);
	if (ret < 0){
        printf("ioctl DIAG_IOCTL_LSM_DEINIT fails, with ret val = %d\n", ret);
    	perror("ioctl DIAG_IOCTL_LSM_DEINIT");
    }
    */

    // ret = ioctl(fd, DIAG_IOCTL_DCI_CLEAR_LOGS, (char *) &client_id);  
    // if (ret < 0){
    //     printf("ioctl DIAG_IOCTL_DCI_CLEAR_LOGS fails, with ret val = %d\n", ret);
    // 	perror("ioctl DIAG_IOCTL_DCI_CLEAR_LOGS");
    // }
    // ret = ioctl(fd, DIAG_IOCTL_DCI_CLEAR_EVENTS, (char *) &client_id);  
    // if (ret < 0){
    //     printf("ioctl DIAG_IOCTL_DCI_CLEAR_EVENTS fails, with ret val = %d\n", ret);
    // 	perror("ioctl DIAG_IOCTL_DCI_CLEAR_EVENTS");
    // }

    /*
     * EXPERIMENTAL (NEXUS 6 ONLY): configure the buffering mode to circular
     */
    struct diag_buffering_mode_t buffering_mode;
    // buffering_mode.peripheral = remote_dev;
    buffering_mode.peripheral = 0;
    buffering_mode.mode = DIAG_BUFFERING_MODE_STREAMING;
    buffering_mode.high_wm_val = DEFAULT_HIGH_WM_VAL;
    buffering_mode.low_wm_val = DEFAULT_LOW_WM_VAL;

    ret = ioctl(fd, DIAG_IOCTL_PERIPHERAL_BUF_CONFIG, (char *) &buffering_mode);  
    if (ret < 0){
        printf("ioctl DIAG_IOCTL_PERIPHERAL_BUF_CONFIG fails, with ret val = %d\n", ret);
    	perror("ioctl DIAG_IOCTL_PERIPHERAL_BUF_CONFIG");
    }
    // uint8_t peripheral = 0;
    // for(;peripheral<=LAST_PERIPHERAL; peripheral++)
    // {
    // 	ret = ioctl(fd, DIAG_IOCTL_PERIPHERAL_BUF_DRAIN, (char *) &peripheral);  
	   //  if (ret < 0){
	   //      printf("ioctl DIAG_IOCTL_PERIPHERAL_BUF_DRAIN fails, with ret val = %d\n", ret);
	   //  	perror("ioctl DIAG_IOCTL_PERIPHERAL_BUF_DRAIN");
	   //  }

	   //  /*
	   //   * EXPERIMENTAL (NEXUS 6 ONLY): configure the buffering mode to circular
	   //   */
	   //  struct diag_buffering_mode_t buffering_mode;
	   //  buffering_mode.peripheral = peripheral;
	   //  buffering_mode.mode = DIAG_BUFFERING_MODE_STREAMING;
	   //  buffering_mode.high_wm_val = DEFAULT_HIGH_WM_VAL;
	   //  buffering_mode.low_wm_val = DEFAULT_LOW_WM_VAL;

	   //  ret = ioctl(fd, DIAG_IOCTL_PERIPHERAL_BUF_CONFIG, (char *) &buffering_mode);  
	   //  if (ret < 0){
	   //      printf("ioctl DIAG_IOCTL_PERIPHERAL_BUF_CONFIG fails, with ret val = %d\n", ret);
	   //  	perror("ioctl DIAG_IOCTL_PERIPHERAL_BUF_CONFIG");
	   //  }
    // }

	

	// /*
	//  * Enable logging mode
	//  * Reference: https://android.googlesource.com/kernel/msm.git/+/android-6.0.0_r0.9/drivers/char/diag/diagchar_core.c
	//  */
	// ret = ioctl(fd, DIAG_IOCTL_SWITCH_LOGGING, (char *) &mode);  
	// if (ret < 0) {
	// 	LOGD("ioctl SWITCH_LOGGING fails: %s \n", strerror(errno));
	// 	perror("ioctl SWITCH_LOGGING");
	// 	// Yuanjie: the following works for Samsung S5
	// 	ret = ioctl(fd, DIAG_IOCTL_SWITCH_LOGGING, (char *) mode);
	// 	if (ret < 0) {
	// 		LOGD("Alternative ioctl SWITCH_LOGGING fails: %s \n", strerror(errno));
	// 		perror("Alternative ioctl SWITCH_LOGGING");

	// 		/* Android 7.0 mode
	// 		 * Reference: https://android.googlesource.com/kernel/msm.git/+/android-7.1.0_r0.3/drivers/char/diag/diagchar_core.c
	// 		 */

	// 		struct diag_logging_mode_param_t new_mode;
	// 		new_mode.req_mode = mode;
	// 		new_mode.peripheral_mask = DIAG_CON_ALL;
	// 		new_mode.mode_param = 0;

	// 		ret = ioctl(fd, DIAG_IOCTL_SWITCH_LOGGING, (char *)& new_mode);
	// 		if (ret < 0) {
	// 			LOGD("Android-7.0 ioctl SWITCH_LOGGING fails: %s \n", strerror(errno));
	// 			perror("Alternative ioctl SWITCH_LOGGING");


	// 			ret = ioctl(fd, DIAG_IOCTL_SWITCH_LOGGING, &mode, 12, 0, 0, 0, 0);
	// 			if (ret < 0) {
	// 				LOGD("S7 Edge fails: %s \n", strerror(errno));
	// 		    }
	// 		}	

	// 	}

	// }
	// else{
	// 	// printf("Older way of ioctl succeeds.\n");
	// }

    /*
     * Enable logging mode
     */
    ret = -1;
    if (ret < 0) {
        // Reference: https://android.googlesource.com/kernel/msm.git/+/android-6.0.0_r0.9/drivers/char/diag/diagchar_core.c
	    ret = ioctl(fd, DIAG_IOCTL_SWITCH_LOGGING, (char *) &mode);
    }
    if (ret < 0) {
        LOGD("ioctl SWITCH_LOGGING fails: %s \n", strerror(errno));
        perror("ioctl SWITCH_LOGGING");
        // Yuanjie: the following works for Samsung S5
        ret = ioctl(fd, DIAG_IOCTL_SWITCH_LOGGING, (char *) mode);
    }
    if (ret < 0) {
        LOGD("Alternative ioctl SWITCH_LOGGING fails: %s \n", strerror(errno));
        perror("Alternative ioctl SWITCH_LOGGING");
        /* Android 7.0 mode
         * * Reference: https://android.googlesource.com/kernel/msm.git/+/android-7.1.0_r0.3/drivers/char/diag/diagchar_core.c
         * */
        struct diag_logging_mode_param_t new_mode;
        new_mode.req_mode = mode;
        new_mode.peripheral_mask = DIAG_CON_ALL;
        new_mode.mode_param = 0;
        ret = ioctl(fd, DIAG_IOCTL_SWITCH_LOGGING, (char *)& new_mode);
    }
    if (ret < 0) {
        LOGD("Android-7.0 ioctl SWITCH_LOGGING fails: %s \n", strerror(errno));
        perror("Alternative ioctl SWITCH_LOGGING");
        // Yuanjie: the following is used for Samsung S7 Edge
        ret = ioctl(fd, DIAG_IOCTL_SWITCH_LOGGING, &mode, 12, 0, 0, 0, 0);
    }
    if (ret < 0) {
        LOGD("S7 Edge ioctl SWITCH_LOGGING fails: %s \n", strerror(errno));
        perror("Alternative ioctl SWITCH_LOGGING");
        // Haotian: try for XiaoMI 6 7.1.1
        ret = ioctl(fd, DIAG_IOCTL_SWITCH_LOGGING, mode);
    }
    if (ret < 0) {
        LOGD("XiaoMI method 1 ioctl SWITCH_LOGGING fails: %s \n", strerror(errno));
        perror("Alternative ioctl SWITCH_LOGGING");
        // Haotian: try for XiaoMI 6 from Yuanjie (32 bits libdiag.so)
        ret = ioctl(fd, DIAG_IOCTL_SWITCH_LOGGING, mode, 0, 0, 0);
    }
    if (ret < 0) {
        LOGD("XiaoMI method 2 ioctl SWITCH_LOGGING fails: %s \n", strerror(errno));
        perror("Alternative ioctl SWITCH_LOGGING");
    }

    if (ret >= 0) {
        LOGD("Enable logging mode success.\n");
    } else {
        LOGD("Failed to enable logging mode.\n");
    }

	return ret;
}

/* function to write into device*/
int  diag_com_write_command(int fd,u_int8_t deviceType,u_int16_t items[])
{
	int writing_status;
	char *write_buf;
	int len;
	write_buf = (char *)MALLOC(1024);
	if(gManufactureType == SAMSUNG){
		write_buf[0]= 0x20;
	}else
		write_buf[0]= 0x40;
	write_buf[1]= 0;
	write_buf[2] =0;
	write_buf[3] =0;
	len= cmd_log_config_set_mask_new (&write_buf[4],1020,deviceType,items);
	d_log("Diag_com_write_command");
	//d_arraylog(write_buf , len+4);
	writing_status = write(fd, write_buf, len+4);
	if (writing_status == -1){
		d_warning("Diag_com_write_event : Writing error to Diag Device %d ",errno);
	}
	FREE(write_buf);
	return writing_status;
}

/*function to  write the event into device*/
int diag_com_write_event(int fd,u_int16_t wcdma_event_item[])
{
	int writing_status;
	char *write_buf;
	int len;
	write_buf = (char *)MALLOC(1024);
	if(gManufactureType == SAMSUNG){
		write_buf[0]= 0x20;
	}else
		write_buf[0]= 0x40;
	write_buf[1]= 0;
	write_buf[2] =0;
	write_buf[3] =0;
	len = cdm_cmd_event_report_new (&write_buf[4],1020,TRUE);/* command  to enable all events*/
	d_log("diag_com_write_event");
	d_arraylog(write_buf , len+4);
	writing_status = write(fd, write_buf, len+4);
	if (writing_status == -1){
		d_warning("Diag_com_write_event : Writing error,Disable event %d ",errno);
	}
	sleep(1);
	memset(&write_buf[4],0,1020);
	len = cdm_cmd_event_mask(&write_buf[4],1020,wcdma_event_item);/*method to configure the event*/
	d_arraylog(write_buf , len);
	writing_status = write(fd, write_buf, len+4);
	if (writing_status == -1){
		d_warning("Diag_com_write_event : Writing error,Enable event %d ",errno);
	}
	FREE(write_buf);
	return writing_status;
}

/*Function to write time request on the device */

 int Dm_cmd_time_request(int fd)
{
	int writing_status;
	char *write_buf;
	int len = 0;
	write_buf = (char *)MALLOC(1024);
	if(gManufactureType == SAMSUNG){
		write_buf[0]= 0x20;
	}else
		write_buf[0]= 0x40;
	write_buf[1]= 0;
	write_buf[2] =0;
	write_buf[3] =0;
	len= cmd_dm_time_request(&write_buf[4],1020);
	d_arraylog(write_buf , len+4);
	writing_status = write(fd, write_buf, len+4);
	if (writing_status == -1){
		d_warning("Diag_Dm_cmd_time_request ErrorNo =  %d ",errno);
	}
	FREE(write_buf);
	return writing_status;
}

/* function to read the diag device*/
static int  diag_com_read(int fd,char *read_buf)
{
	unsigned int dataLength;
	time_t currenttime;
	memset(read_buf ,0 ,50000);
	/*time(&currenttime);*/
	/*fprintf(filefdInternal, " Entry Read Buffer  %s \n " , ctime(&currenttime));*/
	dataLength =read(fd ,read_buf ,50000);
	/*time(&currenttime);*/
	/*fprintf(filefdInternal, " Exit Read Buffer  %s \n" , ctime(&currenttime));*/
	if (dataLength == -1){
		d_warning("Diag_com_read : Reading  error  in device  %d ",errno);
		exit(0);
	}
	return dataLength;
}

int dynamic_log_request(int fd)
{
	int writing_status;
	char *write_buf;
	int len;

	write_buf = (char *)MALLOC(512);
	if(gManufactureType == SAMSUNG){
		write_buf[0]= 0x20;
	}else
		write_buf[0]= 0x40;
	write_buf[1]= 0;
	write_buf[2] =0;
	write_buf[3] =0;
	d_log("Dynamic Logging for 0xB0C2\n ");
	len = cmd_hdr_dynamic_Logging_message (&write_buf[4],508, 0xB0C2);
	d_arraylog(write_buf , len);
	writing_status = write(fd, write_buf, len+4);
	if (writing_status == -1){
		d_warning("Dynamic_log_request : Writing error  while version %d ",errno);
	}
	FREE(write_buf);
	return  writing_status;
}

int Gsm_log_on_demand_request(int fd,u_int16_t cmdcode)
{
	int writing_status;
	char *write_buf;
	int len;
	write_buf = (char *)MALLOC(512);
	if(gManufactureType == SAMSUNG){
		write_buf[0]= 0x20;
	}else
		write_buf[0]= 0x40;
	write_buf[1]= 0;
	write_buf[2] =0;
	write_buf[3] =0;
	d_log("Gsm Log On Demand Request Cmd");
	len = cmd_hdr_Log_On_Demand_Request_Logging_message(&write_buf[4],508, cmdcode);
	d_arraylog(write_buf , len);
	writing_status = write(fd, write_buf, len+4);
	if (writing_status == -1){
		d_warning("Gsm_log_on_demand_request : Writing error = %d ",errno);
	}d_arraylog(write_buf , len+4);
	FREE(write_buf);
	return  writing_status;

}

void  diag_put_efs_request(int fd , u_int32_t data_len, char * data, char * path)
{
	int status;
	char *write_buf;
	int len;
	int path_len  ;
	write_buf = (char *)MALLOC(1024);
	if(gManufactureType == SAMSUNG){
		write_buf[0]= 0x20;
	}else
		write_buf[0]= 0x40;
	write_buf[1]= 0;
	write_buf[2] =0;
	write_buf[3] =0;
	/*prototype -> size_t cmd_efs_diag_put(char *buf, size_t len, u_int32_t data_len, char * data, char * path);*/
	path_len =  strlen(path)+1;
	len = cmd_efs_diag_put (&write_buf[4],1020, data_len,  path_len ,data, path);
	status = write(fd, write_buf, len+4);
	if (status ==  -1){
		d_warning("diag_put_efs_request : writing error =%d ",errno);
	}
	FREE(write_buf);
}

 /* void  diag_get_efs_request( globalFD)*/
 void  diag_get_efs_request(int fd , u_int32_t data_len,  char * path,u_int16_t seqNumber)
{
	int status;
	char *write_buf;
	int len;
	int path_len  ;
	write_buf = (char *)MALLOC(1024);
	if(gManufactureType == SAMSUNG){
		write_buf[0]= 0x20;
	}else
		write_buf[0]= 0x40;
	write_buf[1]= 0;
	write_buf[2] =0;
	write_buf[3] =0;
	path_len =  strlen(path)+1;
	len= cmd_efs_diag_get (&write_buf[4],1020,data_len,path_len, path,seqNumber);
	d_arraylog(write_buf,len+4);
	status = write(fd, write_buf, len+4);
	if (status == -1){
		d_warning("diag_get_efs_request : writing error = %d ",errno);
	}
	FREE(write_buf);
}

void
diag_process_data(queue_t q)
{
	PacketInfo pkt;
	char read_buf[50000];
	unsigned int dataLength;
        unsigned int *logtype = NULL;

	/* call the diag_com_read function to read the data*/
	dataLength = diag_com_read(globalFD, read_buf);
	d_log("diag_process_data");
	d_arraylog(read_buf , dataLength);
	logtype = (int*)&read_buf[0];
	if((*logtype == 32) || (*logtype == 64)) 
	{
		pkt.length = dataLength;
		pkt.msgPtr =  MALLOC(pkt.length);
		memcpy(pkt.msgPtr,read_buf,dataLength);
		if(sendQueue(q, pkt)!=0)
		{
			d_warning("Unable to Send Packet to Processing thread");
			Log_Trace (__FILE__, __LINE__, __func__, LOGL_WARN,
				"Unable to Send Packet to Processing thread");
			FREE(pkt.msgPtr);
		}	
	}		
}

/* function to listen the event on this fd*/
void *diag_com_listen(void * arg)
{
	queue_t q = (queue_t) arg; // We have the Queue now

	fd_set set;
	struct timeval enter , exit;
	struct timeval timeout;
	int retval = 0;
	/* select returns 0 if timeout, 
	                  1 if input atimeoutvailable, 
			 -1 if error. 
	*/
	while (1)
	{
		/* Initialize the file descriptor set. */
		/* Initialize the timeout data structure. */
		timeout.tv_sec = 0;
		timeout.tv_usec = 10*1000;
		FD_ZERO (&set);
		FD_SET (globalFD, &set);
		
		if(!init_done){
			d_log("Exiting Read thread ");
			return NULL;
		} /*This is existing the thread*/
		gettimeofday(&enter, NULL);
		retval = select(globalFD+1 ,&set,NULL ,NULL ,&timeout);
		if (!init_done) {
			d_log("Exiting Read thread ");
			return NULL;
		} /*This is existing the thread*/
		if (retval == -1){
			d_warning("error in select()");
		}else if (retval > 0) {
			if(FD_ISSET(globalFD, &set)) {
				diag_process_data(q);
			}
		}
		else if (retval == 0) {
			d_log("System come out from Select with timeout ");
		}
		gettimeofday(&exit, NULL);
		if (((exit.tv_sec-enter.tv_sec) >= 1)
			&& ((exit.tv_usec-enter.tv_usec) >200000)) {

			d_warning("Warning : DATA COLLECTION THREAD "
				"sec = %d usec %d \n", 
				(exit.tv_sec - enter.tv_sec),
				(exit.tv_usec - enter.tv_usec));
		}
	}

	return NULL;
}


int  diag_com_close(int fd)
{
	init_done = 0;
	close(fd);
}

int diaginit_diag_com_write_command(u_int8_t deviceType,u_int16_t items[])
{
	return diag_com_write_command(globalFD,deviceType,items);
}
int diaginit_diag_com_write_event(u_int16_t event_item[]){
	return  diag_com_write_event(globalFD,event_item);
}

int  diag_com_gps_write_command(int fd)
{
	int writing_status;
	char *write_buf;
	int len;
	write_buf = (char *)MALLOC(1024);
	if(gManufactureType == SAMSUNG){
		write_buf[0]= 0x20;
	}else
		write_buf[0]= 0x40;
	write_buf[1]= 0;
	write_buf[2] =0;
	write_buf[3] =0;
	len= cmd_gps_last_location(&write_buf[4],1020);
	writing_status = write(fd, write_buf, len+4);
	d_log(" GPS Command write ");
	d_arraylog(write_buf,len+4);
	if (writing_status == -1){
		d_warning("diag_com_gps_write_command : Writing error = %d ",errno);
	}
	FREE(write_buf);
	Log_Trace (__FILE__,__LINE__,__func__,LOGL_SUCCESS,"writing_status = %d ",writing_status);
	return writing_status;
}



void diag_com_cdma_request(int fd,u_int16_t code){
	int writing_status;
	char *write_buf;
	int len;
	write_buf = (char *)MALLOC(1024);
	if(gManufactureType == SAMSUNG)
	{
		//printf("\n in diag_com_cdma_request \n");
		write_buf[0]= 0x20;
		write_buf[1]= 0;
		write_buf[2] =0;
		write_buf[3] =0;
		len= cdma_cmd_write(&write_buf[4],1020,code);
		writing_status = write(fd, write_buf, len+4);
		if (writing_status == -1)
		{
			printf("Writing error to Diag Device %d \n",errno);
		}
		FREE(write_buf);
	}
}
