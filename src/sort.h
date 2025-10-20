/***************************************************
*  ___________.__       .___           .__         *
*  \_   _____/|  |    __| _/___________|__|____    *
*   |    __)_ |  |   / __ |/  _ \_  __ \  \__  \   *
*   |        \|  |__/ /_/ (  <_> )  | \/  |/ __ \_ *
*  /_______  /|____/\____ |\____/|__|  |__(____  / *
*          \/            \/                    \/  *
****************************************************/
//
//                Eldoria MUD
//   Bringing the magic of ROM 2.4b6 to 2025!
//
//            Ro Black <icewindlegacy@gmail.com>
//
#ifndef SORT_H_
#define SORT_H_

int issort(void *data, int size, int esize, int (*compare)(const void *key1,
                                                           const void 
*key2));

int qksort(void *data, int size, int esize, int i, int k, int (*compare)
           (const void *key1, const void *key2));

#endif //SORT_H_
