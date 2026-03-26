#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <time.h>

#include <confuse.h>

#include "devdrvd.h"
#include "devdrvd_communication.h"

extern volatile sig_atomic_t shutdown_flag; 
extern int fast_flag;
extern struct devdrvd_client client_pool[];

extern  struct timeval get_time();

static volatile int	iStartWeight=0;

typedef struct {
	int channel;
	int	cnt_weight_intervals;
	int* weight_intervals;
	int* weight_steps;
	int	time_step;
	int	current_weight;
	pthread_t	thread;
} channel_weight_t;

cfg_opt_t static_opts[] = {
	CFG_INT("chan", 0, CFGF_NODEFAULT),
    CFG_INT_LIST("weight_intervals", (char*)"{0}", CFGF_NODEFAULT),
    CFG_INT_LIST("weight_steps", (char*)"{0}", CFGF_NODEFAULT),
    CFG_INT("time_step", 0, CFGF_NODEFAULT),
    CFG_END()
};

cfg_opt_t config_opts[] = {
	CFG_SEC("static", static_opts, CFGF_MULTI),
    CFG_END()
};

static void handlerSigUsr1(int sig)
{
	printf( "handlerSigUsr1\n");
	iStartWeight=1;
}

void emu_scale( void *data)
{
	channel_weight_t* cw = (channel_weight_t*)data;
    struct weight_struct w_s;
    unsigned long timestamp=0;
    struct timeval tv_w;
    char* package = 0;
    size_t package_len = 0;
    struct timespec tv;
    int	idx_interval=0;

	cw->current_weight = cw->weight_intervals[0];

	printf( "Start emu_scale: channel=%d,shut=%d\n", cw->channel, shutdown_flag);
		
	while( shutdown_flag != 1) {
printf( "iStartWeight=%d\n", iStartWeight);	
	   	if( !iStartWeight) {
            tv.tv_sec = 0;
            tv.tv_nsec = 100 * 1000000;
       		nanosleep(&tv, 0);
       		continue;
       	}

printf( "packet\n");
		tv_w.tv_sec = timestamp/1000;
		tv_w.tv_usec = (timestamp%1000) * 1000.0;
        w_s.ts = tv_w;
        w_s.channel = cw->channel;
        w_s.status  = 0;
        w_s.tare    = cw->current_weight;
        w_s.weight  = cw->current_weight;

        package_len = pack_weight(&package, &w_s);

printf( "packet: w=%f\n", w_s.weight);

        /** Dispach data to registered clients */
         int i = 0;
         for(i = 0; i < DEVDRVD_MAX_CLIENTS; ++i) {
         	pthread_mutex_lock(&client_pool[i].lock);
            if(client_pool[i].socket_fd != -1)
            	write(client_pool[i].write_fd, package, package_len);
            pthread_mutex_unlock(&client_pool[i].lock);
         }

         free(package);
         timestamp += cw->time_step;
         if( cw->current_weight <= cw->weight_intervals[cw->cnt_weight_intervals-1]) {
         	cw->current_weight += cw->weight_steps[idx_interval];
         }
         if( (idx_interval+1) < cw->cnt_weight_intervals) {
         	if( cw->current_weight > cw->weight_intervals[idx_interval+1]) {
         		idx_interval++;
         	}
         }
 		
         tv.tv_sec = 0;
         tv.tv_nsec = cw->time_step * 1000000;
		 nanosleep(&tv, 0);
	}
 		
}



int	static_emulation_loop( char* logDir, char* static_cfg)
{
	int	rc=0;
	channel_weight_t* channel_weight = NULL;
	
    DIR *dpdf = NULL;
    struct dirent *epdf;
    int rewind = 0;

    char* package = 0;
    size_t package_len = 0;


    struct weight_struct w_s;
    struct discrete_struct d_s;

	int cnt_channels = 0;

	unsigned long timestamp_w=0;
	
    printf("static emulation_loop(0x%p; 0x%p) called.\n", logDir, static_cfg);

    if(logDir != 0) {
        dpdf = opendir(logDir);
    }
    
    cfg_t *cfg;
    cfg = cfg_init(config_opts, CFGF_NONE);


    /** init gpios/reinit gpios*/
    int status = cfg_parse(cfg, static_cfg);

    switch(status) {
    case CFG_FILE_ERROR:
    	printf( "CFG_FILE_ERROR: cfg_parse()\n");
        return -1;
    case CFG_PARSE_ERROR:
    	printf( "CFG_PARSE_ERROR: cfg_parse()\n");
        return -1;
    case CFG_SUCCESS:
        break;
    }
    
    int	cnt_channel = (int)cfg_size( cfg, "static");
    if( cnt_channel > 0) {
    	channel_weight = calloc( sizeof(channel_weight_t), cnt_channel);
    	if( channel_weight == NULL) {
    		printf( "Error calloc for channel_weight\n");
    		return -1;
    	}
    }
    else {
    	printf( "Not found section 'static'\n");
    	return 0;
    }
	for (int j = 0; j < cnt_channel; j++) {
        cfg_t* static_sec = cfg_getnsec(cfg, "static", j);
		if( static_sec) {
			channel_weight[j].channel = cfg_getint( static_sec, "chan");
			channel_weight[j].cnt_weight_intervals = cfg_size( static_sec, "weight_intervals");
			if( channel_weight[j].cnt_weight_intervals > 0) {
				channel_weight[j].weight_intervals = calloc( sizeof(int), channel_weight[j].cnt_weight_intervals);
				for( int k=0; k < channel_weight[j].cnt_weight_intervals; ++k) {
					channel_weight[j].weight_intervals[k] = cfg_getnint( static_sec, "weight_intervals", k);
				}  
			}
			else {
				printf( "ERROR: No weight intervals\n");
				return -1;
			}
			int cnt_weight_step = cfg_size( static_sec, "weight_steps");
			if( cnt_weight_step < (channel_weight[j].cnt_weight_intervals - 1)) {
				printf( "ERROR: cnt_int=%d cnt_ws=%d\n", channel_weight[j].cnt_weight_intervals, cnt_weight_step);
				return -1;
			}
			channel_weight[j].weight_steps = calloc( sizeof(int), cnt_weight_step);
			for( int k=0; k < cnt_weight_step; ++k) {
				channel_weight[j].weight_steps[k] = cfg_getnint( static_sec, "weight_steps", k);
			}  
			
			channel_weight[j].time_step = cfg_getint( static_sec, "time_step");
			channel_weight[j].current_weight = channel_weight[j].weight_intervals[0];
			
			printf( "chan=%d, cnt_int=%d, cnt_ws=%d, time_step=%d, cur_weight=%d\n", 
				channel_weight[j].channel, channel_weight[j].cnt_weight_intervals, cnt_weight_step, 
				channel_weight[j].time_step, channel_weight[j].current_weight);
		}
	}


//<-nik
// Обработчик сигнала на SIGUSR1
	iStartWeight = 0;
	if( signal(SIGUSR1, handlerSigUsr1) == SIG_ERR) {
		printf( "emulation_loop: error signal( SIGUSR1) %d(%s)\n", errno, strerror( errno));
		exit( EXIT_FAILURE);
	}


    for( int i = 0; i < cnt_channel; i++) {
    	if(pthread_create(&channel_weight[i].thread, NULL, emu_scale, (void*)&channel_weight[i])) {
    	    perror("Could not create connect thread");
//            syslog(LOG_CRIT, "Could not create connect thread");
             return EXIT_FAILURE;
        }
	}
	
    for( int i = 0; i < cnt_channel; i++) {
    	void *rc;
		pthread_join(channel_weight[i].thread, &rc);
	}
		
	return 0;
}
