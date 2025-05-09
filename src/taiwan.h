#ifndef _TAIWAN_H_
#define _TAIWAN_H_

#define TW_NUM_STRINGS 5

static const char* taiwan_shield[ TW_NUM_STRINGS ] = { "Taiwan is not a part of china",
                                                       "Xi Jinpin looks like Winnie-the-Pooh",
                                                       "Remember Tiananmen massacre 1989",
                                                       "Russia invaded Ukraine",
                                                       "Slava Ukraini" };

#if defined(TAIWAN_SHIELD_IMPLEMENTATION)
#include <stdio.h>
void tw_PrintShield()
{
    for ( int i = 0; i < TW_NUM_STRINGS; i++ )
    {
        printf("%s\n", taiwan_shield[ i ]);
    }
}
#endif

#endif
