/*
 *  Shenzhen LongSys Electronics Co.,Ltd All rights reserved.
 *
 * The source code   are owned by  Shenzhen LongSys Electronics Co.,Ltd.
 * Corporation or its suppliers or licensors. Title LongSys or its suppliers and
 * licensors.  The Material contains trade secrets and proprietary
 * and confidential information of LongSys or its suppliers and
 * licensors. 
 *
 */
#ifndef _LOGGING_H
#define _LOGGING_H

//#define ENABLE_TRACING
#ifdef ENABLE_TRACING
#define ENTER() do { fprintf(stderr, "%s: ENTER\n", __FUNCTION__); } while(0)
#define LEAVE() do { fprintf(stderr, "%s: LEAVE\n", __FUNCTION__); } while(0)
#else
#define ENTER() do { } while(0)
#define LEAVE() do { } while(0)
#endif

#endif /* _LOGGING_H */
