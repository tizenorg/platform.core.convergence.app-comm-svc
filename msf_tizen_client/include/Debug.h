#ifndef __DEF_MSF_DBG_H_
#define __DEF_MSF_DBG_H_

#ifndef LINUX
#include <dlog.h>
#undef LOG_TAG
#define LOG_TAG "MSF_API"
#define MSF_DBG(format, args...) SLOGD(format, ##args)
#define MSF_ERR(format, args...) SLOGE(format, ##args)
#endif

#ifdef LINUX
#define MSF_DBG printf
#define MSF_ERR printf
#endif

#endif

