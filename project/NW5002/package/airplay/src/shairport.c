/*
 * Shairport, an Apple Airplay receiver
 * Copyright (c) James Laird 2013
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <getopt.h>
#include <pthread.h>
#include <errno.h>

#include <openssl/md5.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#include "common.h"
#include "daemon.h"
#include "rtsp.h"
#include "mdns.h"
#include "getopt_long.h"
#include "metadata.h"
#include "player.h"

#include "config.h"
//#include "utils_interface.h"
#include "airplay_interface.h"
//#include "sharememory_interface.h"
//#include "player_ipc_interface.h"

static pthread_t shairport_service_thread;
static pthread_t shairport_recv_msg_thread;
static const char *version =
    #include "version.h"
    ;

static void log_setup();

static int shutting_down = 0;

static char apname_s[20 + 100] = {0};
char myssid[64]={0};
char mymac[64]={0};
char myport[64]={0};

extern int *sockfd;
extern int nsock;
extern pthread_t disconnect_airplay_thread;

#ifdef CONFIG_MUTEBUFFER_ADJUST
static int adjust_flag = 0;
static int mute_add = 0;
static int mute_drop = 0;
static int drop_gate = 0;
extern void audio_set_adjust(int mute_add, int mute_drop, int drop_gate);
#endif

static void release_sockets(void)
{
    int i = 0;

    if (sockfd) {
        for(i = 0; i < nsock; i++) {
            close(sockfd[i]);
        }

        free(sockfd);
    }

    sockfd = NULL;
    nsock = 0;
}

void shairport_shutdown(int retval) {
    if (shutting_down)
        return;
    shutting_down = 1;
    printf("11Shairport Shutting down...\n");

    release_sockets();


    mdns_unregister();

    daemon_exit(); // This does nothing if not in daemon mode

    //we do not exit really.
    exit(retval);
}

static void sig_ignore(int foo, siginfo_t *bar, void *baz) {
}
static void sig_shutdown(int foo, siginfo_t *bar, void *baz) {
	printf("sig_shutdown\n");
    shairport_shutdown(0);
}

static void sig_child(int foo, siginfo_t *bar, void *baz) {
    pid_t pid;
    while ((pid = waitpid((pid_t)-1, 0, WNOHANG)) > 0) {
        if (pid == mdns_pid && !shutting_down) {
            die("MDNS child process died unexpectedly!");
        }
    }
}

static void sig_logrotate(int foo, siginfo_t *bar, void *baz) {
    log_setup();
}

void usage(char *progname) {
    printf("Usage: %s [options...]\n", progname);
    printf("  or:  %s [options...] -- [audio output-specific options]\n", progname);

    printf("\n");
    printf("Mandatory arguments to long options are mandatory for short options too.\n");

    printf("\n");
    printf("Options:\n");
    printf("    -h, --help          show this help\n");
    printf("    -p, --port=PORT     set RTSP listening port\n");
    printf("    -a, --name=NAME     set advertised name\n");
    printf("    -t, --rtsp-time=SECONDS set RTSP connect alive time out\n");
    printf("                        valid >= 6s, or Close RTSP time out\n");
    printf("    -k, --password=PW   require password to stream audio\n");
    printf("    -b FILL             set how full the buffer must be before audio output\n");
    printf("                        starts. This value is in frames; default %d\n", config.buffer_start_fill);
    printf("    -d, --daemon        fork (daemonise). The PID of the child process is\n");
    printf("                        written to stdout, unless a pidfile is used.\n");
    printf("    -P, --pidfile=FILE  write daemon's pid to FILE on startup.\n");
    printf("                        Has no effect if -d is not specified\n");
    printf("    -l, --log=FILE      redirect shairport's standard output to FILE\n");
    printf("                        If --error is not specified, it also redirects\n");
    printf("                        error output to FILE\n");
    printf("    -e, --error=FILE    redirect shairport's standard error output to FILE\n");
    printf("    -B, --on-start=COMMAND  run a shell command when playback begins\n");
    printf("    -E, --on-stop=COMMAND   run a shell command when playback ends\n");
    printf("    -w, --wait-cmd          block while the shell command(s) run\n");
    printf("    -M, --meta-dir=DIR      set a directory to write metadata and album cover art to\n");
#ifdef CONFIG_MUTEBUFFER_ADJUST
    printf("\n""    -A, --add-mute      set add mute buffer numbers for buff underrun.\n"
           "                        It has a default value.\n");
    printf("    -D, --drop-mute     set drop mute buffer numbers for buff overrun.\n"
           "                        It has a default value.\n");
    printf("    -G, --drop-gate     set drop mute gate numbers for start mute drop.\n"
           "                        It has a default value.\n\n");
#endif
    printf("    -o, --output=BACKEND    select audio output method\n");
    printf("    -m, --mdns=BACKEND      force the use of BACKEND to advertise the service\n");
    printf("                            if no mdns provider is specified,\n");
    printf("                            shairport tries them all until one works.\n");
	printf("    -S, --ssid=SSID         airmusic self ssid.\n");
	printf("    -M, --mac=MAC           airmusic self mac.\n");
	printf("    -O, --sport=SPORT        airmusic self port.\n");

    printf("\n");
    mdns_ls_backends();
    printf("\n");
  //  audio_ls_outputs();
}

int parse_options(int argc, char **argv) {
    // prevent unrecognised arguments from being shunted to the audio driver
    setenv("POSIXLY_CORRECT", "", 1);

    //recover to default value, so we can invoke getopt() many times.
    optind = 1;

    static struct option long_options[] = {
        {"help",      no_argument,        NULL, 'h'},
        {"daemon",    no_argument,        NULL, 'd'},
        {"pidfile",   required_argument,  NULL, 'P'},
        {"log",       required_argument,  NULL, 'l'},
        {"error",     required_argument,  NULL, 'e'},
        {"port",      required_argument,  NULL, 'p'},
        {"rtsp-time", required_argument,  NULL, 't'},
        {"name",      required_argument,  NULL, 'a'},
        {"password",  required_argument,  NULL, 'k'},
        {"output",    required_argument,  NULL, 'o'},
        {"on-start",  required_argument,  NULL, 'B'},
        {"on-stop",   required_argument,  NULL, 'E'},
        {"wait-cmd",  no_argument,        NULL, 'w'},
        {"ssid", required_argument,  NULL, 'S'},
		{"mac", required_argument,  NULL, 'M'},
        {"meta-dir",  required_argument,  NULL, 'M'},
        {"sport", required_argument,  NULL, 'O'},
        {"mdns",      required_argument,  NULL, 'm'},
#ifdef CONFIG_MUTEBUFFER_ADJUST
	{"add-mute",  required_argument,  NULL, 'A'},
	{"drop-mute", required_argument,  NULL, 'D'},
	{"drop-gate", required_argument,  NULL, 'G'},
#endif
        {"Verbose",   required_argument,  NULL, 'V'},
        {NULL,        0,                  NULL,   0},
    };

    int opt;
    while ((opt = getopt_long(argc, argv,
                              "+hdV:P:l:e:p:t:a:k:o:b:B:E:M:wm:S:M:O:"
#ifdef CONFIG_MUTEBUFFER_ADJUST
                              "A:D:G:"
#endif
                              ,
                              long_options, NULL)) > 0) {
        switch (opt) {
            default:
                usage(argv[0]);
                exit(1);
            case 'h':
                usage(argv[0]);
                exit(0);
            case 'd':
                config.daemonise = 1;
                break;
            case 'V':
                if (atoi(optarg) > 0)
                    debuglev = atoi(optarg);
                break;
            case 'p':
                config.port = atoi(optarg);
                break;
            case 't':
                if (atoi(optarg) < 6) {
                    printf("Set RTSP time less than 6 seconds, RTSP without time out.\n");
                    config.rtsp_time = 0;
                    break;
                }
		config.rtsp_time = atoi(optarg);
		break;
            case 'a':
                config.apname = optarg;
                break;
            case 'o':
                config.output_name = optarg;
                break;
            case 'k':
                config.password = optarg;
                break;
            case 'b':
                config.buffer_start_fill = atoi(optarg);
                break;
            case 'B':
                config.cmd_start = optarg;
                break;
            case 'E':
                config.cmd_stop = optarg;
                break;
            case 'w':
                config.cmd_blocking = 1;
                break;
            //case 'M':
              //  config.meta_dir = optarg;
                break;
            case 'P':
                config.pidfile = optarg;
                break;
            case 'l':
                config.logfile = optarg;
                break;
            case 'e':
                config.errfile = optarg;
                break;
            case 'm':
                config.mdns_name = optarg;
                break;
#ifdef CONFIG_MUTEBUFFER_ADJUST
            case 'A':
		adjust_flag = 1;
		mute_add = atoi(optarg);
                break;

            case 'D':
		adjust_flag = 1;
		mute_drop = atoi(optarg);
                break;

            case 'G':
		adjust_flag = 1;
		drop_gate = atoi(optarg);
                break;
#endif
		case 'S':
		    if(optarg)
			{
			    snprintf(myssid,sizeof(myssid),"ssid=%s",optarg);
                config.ssid = myssid;printf("%s\n",config.ssid);
			}
            break;
		case 'M':
			if(optarg)
			{
			    snprintf(mymac,sizeof(mymac),"mac=%s",optarg);
                config.mac = mymac;printf("%s\n",config.mac);
			}
            break;
		case 'O':
		    if(optarg)
			{
			    snprintf(myport,sizeof(myport),"Sport=%s",optarg);
                config.sport = myport;printf("%s\n",config.sport);
			}
            break;
        }
    }
    return optind;
}

void signal_setup(void) {
    // mask off all signals before creating threads.
    // this way we control which thread gets which signals.
    // for now, we don't care which thread gets the following.
    sigset_t set;
    sigfillset(&set);

    /* SIGKILL and SIGSTOP cannot be
     * caught, blocked, or ignored.
     */
    sigdelset(&set, SIGINT);
    sigdelset(&set, SIGTERM);
    sigdelset(&set, SIGHUP);
    sigdelset(&set, SIGSTOP);
    sigdelset(&set, SIGCHLD);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    // setting this to SIG_IGN would prevent signalling any threads.
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = &sig_ignore;
    sigaction(SIGUSR1, &sa, NULL);

    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sa.sa_sigaction = &sig_shutdown;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    sa.sa_sigaction = &sig_logrotate;
    sigaction(SIGHUP, &sa, NULL);

    sa.sa_sigaction = &sig_child;
    sigaction(SIGCHLD, &sa, NULL);
}

// forked daemon lets the spawner know it's up and running OK
// should be called only once!
void shairport_startup_complete(void) {
    if (config.daemonise) {
        daemon_ready();
    }
}

void log_setup() {
    if (config.logfile) {
        int log_fd = open(config.logfile,
                O_WRONLY | O_CREAT | O_APPEND,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (log_fd < 0)
            die("Could not open logfile");

        dup2(log_fd, STDOUT_FILENO);
        setvbuf (stdout, NULL, _IOLBF, BUFSIZ);

        if (!config.errfile) {
            dup2(log_fd, STDERR_FILENO);
            setvbuf (stderr, NULL, _IOLBF, BUFSIZ);
        }
    }

    if (config.errfile) {
        int err_fd = open(config.errfile,
                O_WRONLY | O_CREAT | O_APPEND,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (err_fd < 0)
            die("Could not open logfile");

        dup2(err_fd, STDERR_FILENO);
        setvbuf (stderr, NULL, _IOLBF, BUFSIZ);
    }
}

int mozart_airplay_init(int argc, char **argv) {
    printf("Starting Shairport %s\n", version);
    signal_setup();
    memset(&config, 0, sizeof(config));

    // set defaults
    config.buffer_start_fill = 220;
    //config.buffer_start_fill = 100;
    config.port = 5002;
    char hostname[100];
    gethostname(hostname, 100);
    config.apname = apname_s;
    snprintf(config.apname, 20 + 100, "Shairport on %s", hostname);
    config.rtsp_time = 15; /* Default RTSP alive time 15 seconds */

    // parse arguments into config
    int audio_arg = parse_options(argc, argv);

    // mDNS supports maximum of 63-character names (we append 13).
    if (strlen(config.apname) > 50)
        die("Supplied name too long (max 50 characters)");

    if (config.daemonise) {
        daemon_init();
    }

    log_setup();

	#if 0
    config.output = audio_get_output(config.output_name);
    if (!config.output) {
        audio_ls_outputs();
        die("Invalid audio output specified!");
    }
	#endif
   // config.output->init(argc-audio_arg, argv+audio_arg);

    uint8_t ap_md5[MD5_DIGEST_LENGTH]; /* MD5_DIGEST_LENGTH = 16 */
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, config.apname, strlen(config.apname));
    MD5_Final(ap_md5, &ctx);
    memcpy(config.hw_addr, ap_md5, sizeof(config.hw_addr));

    if (config.meta_dir)
        metadata_open();

    return 0;
}

static void *shairport_service_func(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    rtsp_listen_loop();

    return (void *)1; // make compile happy.
}

void mozart_airplay_start_service(void)
{
    if (pthread_create(&shairport_service_thread, NULL, shairport_service_func, NULL) != 0)
        die("Creadte service_thread: %s", strerror(errno));
    pthread_detach(shairport_service_thread);
}

int mozart_airplay_stop_playback(void)
{
    rtsp_shutdown_stream();
    if (config.output)
        config.output->stop();

    return 0;
}

void mozart_airplay_shutdown(void)
{
    mozart_airplay_stop_playback();

    //if(share_mem_set(AIRPLAY_DOMAIN, STATUS_SHUTDOWN))
        printf("share_mem_set failure.\n");
//
    printf("22Shairport Shutting down...\n");

    pthread_cancel(shairport_service_thread);
    //pthread_cancel(shairport_recv_msg_thread);

    mdns_unregister();

    release_sockets();

    daemon_exit(); // This does nothing if not in daemon mode

    return;
}
