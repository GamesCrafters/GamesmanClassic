VALUE StoreValueOfPosition(POSITION position, VALUE value)
{
    VALUE *ptr;
    
    if (gTwoBits) {
        int shamt;
	
        ptr = GetRawValueFromDatabase(position >> 4);
        shamt = (position & 0xf) * 2;
	
        *ptr = (*ptr & ~(0x3 << shamt)) | ((0x3 & value) << shamt);
        return value;
    }
    
    ptr = GetRawValueFromDatabase(position);
    
    /* put it in the right position, but we have to blank field and then
    ** add new value to right slot, keeping old slots */
    return((*ptr = ((*ptr & ~VALUE_MASK) | (value & VALUE_MASK))) & VALUE_MASK); 
}

// This is it
VALUE GetValueOfPosition(POSITION position)
{
    //Gameline code removed
    VALUE *ptr;
    
    if (gTwoBits) {
        /* values are always 32 bits */
        ptr = GetRawValueFromDatabase(position >> 4);
        return (VALUE)(3 & ((int)*ptr >> ((position & 0xf) * 2)));
    } else {
        ptr = GetRawValueFromDatabase(position);
        return((VALUE)((int)*ptr & VALUE_MASK)); /* return pure value */
    }
}


REMOTENESS Remoteness(POSITION position)
{
    //Gameline code removed
    VALUE *GetRawValueFromDatabase(), *ptr;
    if (gTwoBits) {
        return 254;
    } else {
        ptr = GetRawValueFromDatabase(position);
        return((((int)*ptr & REMOTENESS_MASK) >> REMOTENESS_SHIFT));
    }
}

void SetRemoteness (POSITION position, REMOTENESS remoteness)
{
    VALUE *ptr;
    
    if (gTwoBits)
        return;
    
    ptr = GetRawValueFromDatabase(position);
    
    if(remoteness > REMOTENESS_MAX) {
        printf("Remoteness request (%d) for " POSITION_FORMAT  " larger than Max Remoteness (%d)\n",remoteness,position,REMOTENESS_MAX);
        ExitStageRight();
        exit(0);
    }
    
    /* blank field then add new remoteness */
    *ptr = (VALUE)(((int)*ptr & ~REMOTENESS_MASK) | 
		   (remoteness << REMOTENESS_SHIFT));       
}

BOOLEAN Visited(POSITION position)
{
    VALUE *ptr;
    
    if (gVisited)
        return (gVisited[position >> 3] >> (position & 7)) & 1;
    
    if (gTwoBits)
        return FALSE;
    
    ptr = GetRawValueFromDatabase(position);
    
    return((((int)*ptr & VISITED_MASK) == VISITED_MASK)); /* Is bit set? */
}

void MarkAsVisited (POSITION position)
{
    VALUE *ptr;
    
    showStatus(0);
    
    if (gTwoBits) {
        if (gVisited)
            gVisited[position >> 3] |= 1 << (position & 7);
	
        return;
    }
    
    ptr = GetRawValueFromDatabase(position);
    
    *ptr = (VALUE)((int)*ptr | VISITED_MASK);       /* Turn bit on */
}

void UnMarkAsVisited (POSITION position)
{
    VALUE *ptr;
    
    if (gTwoBits) {
        if (gVisited)
            gVisited[position >> 3] &= ~(1 << (position & 7));
	
        return;
    }
    
    ptr = GetRawValueFromDatabase(position);
    
    *ptr = (VALUE)((int)*ptr & ~VISITED_MASK);      /* Turn bit off */
}
