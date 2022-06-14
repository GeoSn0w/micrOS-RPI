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
