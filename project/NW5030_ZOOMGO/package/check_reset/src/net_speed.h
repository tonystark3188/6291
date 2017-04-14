#ifndef __NET_SPEED_H__
#define __NET_SPEED_H__

/******************************************************************************
 *                               INCLUDES                                     *
 ******************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define MAX_BUFF_SIZE 256
#define MAX_INTERFACE 16
struct net_stat {
    char dev[16];
    int up;
    long long last_read_recv, last_read_trans; //last read total num
    long long recv, trans; //real total num
    double recv_speed, trans_speed;
};

int get_net_stat_for_dev(struct net_stat *dev_net_stat, int delta);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
