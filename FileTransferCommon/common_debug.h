#pragma once

#define FLAG_DEBUG 1

#if FLAG_DEBUG==1
#define printd printf
#else
#define printd(...)
#endif