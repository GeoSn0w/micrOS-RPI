typedef unsigned char uint8_t;

void *micrOS_MemoryMove (void *destinationAddr, const void *sourceAddr, unsigned int length) {
    
    register unsigned int realDestination = (unsigned int)destinationAddr;
    register unsigned int realSource = (unsigned int)sourceAddr;

    if(!length)
        return destinationAddr;

    if (realDestination>realSource && realDestination<(realSource+length)) {
        while(length & 3)
        {
            length--;
            ((unsigned char *)realDestination)[length] = ((unsigned char *)realSource)[length];
        }

        while(length)
        {
            length-=4;
            *((unsigned int *)realDestination+length) = *((unsigned int *)realSource+length);
        }
    } else {
        while(length & 0xfffffffc)
        {
            *((unsigned int *)realDestination) = *((unsigned int *)realSource);
            realDestination+=4;
            realSource+=4;
            length-=4;
        }

        while(length) {
            *((unsigned char *)realDestination) = *((unsigned char *)realSource);
            realDestination++;
            realSource++;
            length--;
        }
    }
    return destinationAddr;
}

void *memset(void *dest, unsigned char val, unsigned short len){
    uint8_t *ptr = dest;
    while (len-- > 0)
       *ptr++ = val;
    return dest;
}

void *memcpy(void *dest, const void *src, unsigned short len){
    uint8_t *d = dest;
    const uint8_t *s = src;
    while (len--)
       *d++ = *s++;
    return dest;
}

uint8_t memcmp(void *str1, void *str2, unsigned count){
    uint8_t *s1 = str1;
    uint8_t *s2 = str2;

    while (count-- > 0)
    {
       if (*s1++ != *s2++)
          return s1[-1] < s2[-1] ? -1 : 1;
    }

    return 0;
}
