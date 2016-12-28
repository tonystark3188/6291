/**
 * @file airplay_interface.h
 * @brief For the operation of the AirPlay API
 * @author  czhang <chao.zhang@ingenic.com>
 * @version 1.0.0
 * @date 2015-01-12
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co., Ltd.
 * 
 * The program is not free, Ingenic without permission,
 * no one shall be arbitrarily (including but not limited
 * to: copy, to the illegal way of communication, display,
 * mirror, upload, download) use, or by unconventional
 * methods (such as: malicious intervention Ingenic data)
 * Ingenic's normal service, no one shall be arbitrarily by
 * software the program automatically get Ingenic data
 * Otherwise, Ingenic will be investigated for legal responsibility
 * according to law.
 */

#ifndef _AIRPLAY_H
#define _AIRPLAY_H

/**
 * @brief mozart_airplay_init Airplay initialization function
 *
 * @param argc The number of parameters
 * @param argv The parameter list
 *
 * @return The successful return 0
 */
extern int mozart_airplay_init(int argc, char **argv);

/**
 * @brief mozart_airplay_service_start Start shairport service,such as mDNS、RTSP
 */
extern void mozart_airplay_start_service(void);

/**
 * @brief mozart_airplay_play_stop Stop the shairport music player (call switching DLNA playback)
 */
extern int mozart_airplay_stop_playback(void);

/**
 * @brief mozart_airplay_shutdown Close the exit shairport (call switching network)
 */
extern void mozart_airplay_shutdown(void);

#endif /* _AIRPLAY_H */
