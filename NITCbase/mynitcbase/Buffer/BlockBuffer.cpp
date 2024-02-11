#include "BlockBuffer.h"
#include<cstring>
#include<iostream>
// the declarations for these functions can be found in "BlockBuffer.h"

BlockBuffer::BlockBuffer(int blockNum) {
	// initialise this.blockNum with the argument
	this->blockNum = blockNum;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

/*
Used to get the header of the block into the location pointed to by `head`
NOTE: this function expects the caller to allocate memory for `head`
*/
int BlockBuffer::getHeader(struct HeadInfo* head) {
	unsigned char* bufferPtr;
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);
	if (ret != SUCCESS) {
		return ret;   // return any errors that might have occured in the process
	}

	// populate the numEntries, numAttrs and numSlots fields in *head
	memcpy(&head->numSlots, bufferPtr + 24, 4);
	memcpy(&head->numEntries, bufferPtr + 16, 4);
	memcpy(&head->numAttrs, bufferPtr + 20, 4);
	memcpy(&head->rblock, bufferPtr + 12, 4);
	memcpy(&head->lblock, bufferPtr + 8, 4);

	return SUCCESS;
}

int BlockBuffer::setHeader(struct HeadInfo *head){

    unsigned char *bufferPtr;
    // get the starting address of the buffer containing the block using
    // loadBlockAndGetBufferPtr(&bufferPtr).
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
	if (ret != SUCCESS) {
		return ret;
	}

    // cast bufferPtr to type HeadInfo*
    struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;

    // copy the fields of the HeadInfo pointed to by head (except reserved) to
    // the header of the block (pointed to by bufferHeader)
    //(hint: bufferHeader->numSlots = head->numSlots )
	bufferHeader->blockType = head->blockType;
	bufferHeader->pblock = head->pblock;
	bufferHeader->lblock = head->lblock;
	bufferHeader->rblock = head->rblock;
	bufferHeader->numAttrs = head->numAttrs;
	bufferHeader->numEntries = head->numEntries;
	bufferHeader->numSlots = head->numSlots;

    // update dirty bit by calling StaticBuffer::setDirtyBit()
    // if setDirtyBit() failed, return the error code
	ret = StaticBuffer::setDirtyBit(this->blockNum);
	if (ret != SUCCESS) {
		return ret;
	}

    // return SUCCESS;
	return SUCCESS;
}

/*
Used to get the record at slot `slotNum` into the array `rec`
NOTE: this function expects the caller to allocate memory for `rec`
*/
int RecBuffer::getRecord(union Attribute* rec, int slotNum) {

	// loading the block at this->blockNum into buffer {used here, instead of Disk::readBlock()}
	unsigned char* bufferPtr;
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);
	if (ret != SUCCESS) {
		return ret;
	}

	struct HeadInfo head;

	// get the header using this.getHeader() function
	this->getHeader(&head);

	int attrCount = head.numAttrs;
	int slotCount = head.numSlots;

	/* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
	   - each record will have size attrCount * ATTR_SIZE
	   - slotMap will be of size slotCount
	*/
	int recordSize = attrCount * ATTR_SIZE;
	unsigned char* slotPointer = bufferPtr + HEADER_SIZE + slotCount + (recordSize * slotNum);

	// load the record into the rec data structure
	memcpy(rec, slotPointer, recordSize);

	return SUCCESS;
}

/* NOTE: This function will NOT check if the block has been initialised as a
   record or an index block. It will copy whatever content is there in that
   disk block to the buffer.
   Also ensure that all the methods accessing and updating the block's data
   should call the loadBlockAndGetBufferPtr() function before the access or
   update is done. This is because the block might not be present in the
   buffer due to LRU buffer replacement. So, it will need to be bought back
   to the buffer before any operations can be done.
 */
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char** buffPtr) {
	/* check whether the block is already present in the buffer
	   using StaticBuffer.getBufferNum() */
	int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

	// if present (!=E_BLOCKNOTINBUFFER),
		// set the timestamp of the corresponding buffer to 0 and increment the
		// timestamps of all other occupied buffers in BufferMetaInfo.

	// else
		// get a free buffer using StaticBuffer.getFreeBuffer()

		// if the call returns E_OUTOFBOUND, return E_OUTOFBOUND here as
		// the blockNum is invalid

		// Read the block into the free buffer using readBlock()
	if (bufferNum != E_BLOCKNOTINBUFFER) {
		StaticBuffer::metainfo[bufferNum].timeStamp = 0;

		for (int i = 0; i < BUFFER_CAPACITY; i++) {
			if (i != bufferNum && !StaticBuffer::metainfo[i].free) {
				StaticBuffer::metainfo[i].timeStamp++;
			}
		}
	}
	else {
		bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
		if (bufferNum == E_OUTOFBOUND) {
			return E_OUTOFBOUND;
		}
		Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
	}

	// store the pointer to this buffer (blocks[bufferNum]) in *buffPtr
	*buffPtr = StaticBuffer::blocks[bufferNum];

	// return SUCCESS;
	return SUCCESS;
}

/* used to get the slotmap from a record block
NOTE: this function expects the caller to allocate memory for `*slotMap`
*/
int RecBuffer::getSlotMap(unsigned char* slotMap) {
	unsigned char* bufferPtr;

	// get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);
	if (ret != SUCCESS) {
		return ret;
	}

	struct HeadInfo head;
	// get the header of the block using getHeader() function
	this->getHeader(&head);

	int slotCount = head.numSlots;

	// get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
	unsigned char* slotMapInBuffer = bufferPtr + HEADER_SIZE;

	// copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
	memcpy(slotMap, slotMapInBuffer, slotCount);

	return SUCCESS;
}


int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {

	double diff;

	// if attrType == STRING
	//     diff = strcmp(attr1.sval, attr2.sval)

	// else
	//     diff = attr1.nval - attr2.nval

	if (attrType == STRING) {
		diff = strcmp(attr1.sVal, attr2.sVal);
	}
	else {
		diff = attr1.nVal - attr2.nVal;
	}

	/*
	if diff > 0 then return 1
	if diff < 0 then return -1
	if diff = 0 then return 0
	*/

	if (diff > 0) {
		return 1;
	}
	else if (diff < 0) {
		return -1;
	}
	else {
		return 0;
	}

}

/* Sets the `slotNum`th record entry of the block with the input record contents.
  rec	- Pointer to the array of union Attribute elements from which the record entry is set.
  slotNum	- Slot number of the record in the block.
*/
int RecBuffer::setRecord(union Attribute* rec, int slotNum) {
	unsigned char* bufferPtr;
	/* get the starting address of the buffer containing the block
	   using loadBlockAndGetBufferPtr(&bufferPtr). */
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);

	// if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
		// return the value returned by the call.
	if (ret != SUCCESS) {
		return ret;
	}

	/* get the header of the block using the getHeader() function */
	HeadInfo head;
	this->getHeader(&head);

	// get number of attributes in the block.
	int attrCount = head.numAttrs;

	// get the number of slots in the block.
	int slotCount = head.numSlots;

	// if input slotNum is not in the permitted range return E_OUTOFBOUND.
	if (slotNum >= slotCount || slotNum < 0) {
		return E_OUTOFBOUND;
	}

	/* offset bufferPtr to point to the beginning of the record at required
		slot. the block contains the header, the slotmap, followed by all
		the records. so, for example,
		record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
		copy the record from `rec` to buffer using memcpy
		(hint: a record will be of size ATTR_SIZE * numAttrs)
	*/
	int recordSize = attrCount * ATTR_SIZE;
	unsigned char* slotPointer = bufferPtr + HEADER_SIZE + slotCount + (slotNum * recordSize);

	memcpy(slotPointer, rec, recordSize);

	// update dirty bit using setDirtyBit()
	ret = StaticBuffer::setDirtyBit(this->blockNum);

	/* (the above function call should not fail since the block is already
		in buffer and the blockNum is valid. If the call does fail, there
		exists some other issue in the code) */

		// return SUCCESS

	return SUCCESS;
}

int BlockBuffer::setBlockType(int blockType){

    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
	if (ret != SUCCESS) {
		return ret;
	}

    // store the input block type in the first 4 bytes of the buffer.
    // (hint: cast bufferPtr to int32_t* and then assign it)
    // *((int32_t *)bufferPtr) = blockType;
	*((int32_t *)bufferPtr) = blockType;

    // update the StaticBuffer::blockAllocMap entry corresponding to the
    // object's block number to `blockType`.
	StaticBuffer::blockAllocMap[this->blockNum] = blockType;

    // update dirty bit by calling StaticBuffer::setDirtyBit()
    // if setDirtyBit() failed
        // return the returned value from the call

    // return SUCCESS
	return StaticBuffer::setDirtyBit(this->blockNum);
}

int BlockBuffer::getFreeBlock(int blockType){

    // iterate through the StaticBuffer::blockAllocMap and find the block number
    // of a free block in the disk.
	int blockNum = -1;
	for (int i = 0; i < DISK_BLOCKS; i++) {
		if (StaticBuffer::blockAllocMap[i] == UNUSED_BLK) {
			blockNum = i;
			break;
		}
	}

    // if no block is free, return E_DISKFULL.
	if (blockNum == -1) {
		return E_DISKFULL;
	}

    // set the object's blockNum to the block number of the free block.
	this->blockNum = blockNum;

    // find a free buffer using StaticBuffer::getFreeBuffer() .
	int bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

    // initialize the header of the block passing a struct HeadInfo with values
    // pblock: -1, lblock: -1, rblock: -1, numEntries: 0, numAttrs: 0, numSlots: 0
    // to the setHeader() function.
	HeadInfo head;
	head.pblock = -1;
	head.lblock = -1;
	head.rblock = -1;
	head.numEntries = 0;
	head.numAttrs = 0;
	head.numSlots = 0;
	this->setHeader(&head);

    // update the block type of the block to the input block type using setBlockType().
	this->setBlockType(blockType);

    // return block number of the free block.
	return this->blockNum;
}

BlockBuffer::BlockBuffer(char blockType){
    // allocate a block on the disk and a buffer in memory to hold the new block of
    // given type using getFreeBlock function and get the return error codes if any.
	int ret;
	if (blockType == 'R') {
		ret = getFreeBlock(REC);
	}
	
    // set the blockNum field of the object to that of the allocated block
    // number if the method returned a valid block number,
    // otherwise set the error code returned as the block number.
	this->blockNum = ret;

    // (The caller must check if the constructor allocatted block successfully
    // by checking the value of block number field.)
}

RecBuffer::RecBuffer() : BlockBuffer('R'){}
// call parent non-default constructor with 'R' denoting record block.

/* Sets the slotmap of the block. */
int RecBuffer::setSlotMap(unsigned char *slotMap) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block using
       loadBlockAndGetBufferPtr(&bufferPtr). */
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
	if (ret != SUCCESS) {
		ret;
	}

    // get the header of the block using the getHeader() function
	HeadInfo head;
	getHeader(&head);

	/* the number of slots in the block */
    int numSlots = head.numSlots;

    // the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
    // argument `slotMap` to the buffer replacing the existing slotmap.
    // Note that size of slotmap is `numSlots`
	memcpy(bufferPtr + HEADER_SIZE, slotMap, numSlots);

    // update dirty bit using StaticBuffer::setDirtyBit
    // if setDirtyBit failed, return the value returned by the call

    // return SUCCESS
	return StaticBuffer::setDirtyBit(this->blockNum);
}

/* Returns the block number of the block. Defined to 
access the private member field `blockNum` of the class. */
int BlockBuffer::getBlockNum(){

    //return corresponding block number.
	return this->blockNum;
}