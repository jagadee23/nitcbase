#include "BPlusTree.h"

#include <cstring>

RecId BPlusTree::bPlusSearch(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int op)
{
    // declare searchIndex which will be used to store search index for attrName.
    IndexId searchIndex;

    /* get the search index corresponding to attribute with name attrName
       using AttrCacheTable::getSearchIndex(). */
    AttrCacheTable::getSearchIndex(relId, attrName, &searchIndex);

    AttrCatEntry attrCatEntry;
    /* load the attribute cache entry into attrCatEntry using
     AttrCacheTable::getAttrCatEntry(). */
    AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

    // declare variables block and index which will be used during search
    int block, index;

    /*if searchIndex == {-1, -1}*/
    if (searchIndex.block == -1 && searchIndex.index == -1)
    {
        // (search is done for the first time)

        // start the search from the first entry of root.
        block = attrCatEntry.rootBlock;
        index = 0;

        /* if attrName doesn't have a B+ tree (block == -1)*/
        if (block == -1)
        {
            return RecId{-1, -1};
        }
    }
    else
    {
        /*a valid searchIndex points to an entry in the leaf index of the attribute's
        B+ Tree which had previously satisfied the op for the given attrVal.*/

        block = searchIndex.block;
        index = searchIndex.index + 1; // search is resumed from the next index.

        // load block into leaf using IndLeaf::IndLeaf().
        IndLeaf leaf(block);

        // declare leafHead which will be used to hold the header of leaf.
        HeadInfo leafHead;

        // load header into leafHead using BlockBuffer::getHeader().
        leaf.getHeader(&leafHead);

        if (index >= leafHead.numEntries)
        {
            /* (all the entries in the block has been searched; search from the
            beginning of the next leaf index block. */

            // update block to rblock of current block and index to 0.
            block = leafHead.rblock;
            index = 0;

            if (block == -1)
            {
                // (end of linked list reached - the search is done.)
                return RecId{-1, -1};
            }
        }
    }

    /******  Traverse through all the internal nodes according to value
             of attrVal and the operator op                             ******/

    /* (This section is only needed when
        - search restarts from the root block (when searchIndex is reset by caller)
        - root is not a leaf
        If there was a valid search index, then we are already at a leaf block
        and the test condition in the following loop will fail)
    */

    /* while block is of type IND_INTERNAL */
    while (StaticBuffer::getStaticBlockType(block) == IND_INTERNAL)
    { // use StaticBuffer::getStaticBlockType()

        // load the block into internalBlk using IndInternal::IndInternal().
        IndInternal internalBlk(block);

        HeadInfo intHead;

        // load the header of internalBlk into intHead using BlockBuffer::getHeader()
        internalBlk.getHeader(&intHead);

        // declare intEntry which will be used to store an entry of internalBlk.
        InternalEntry intEntry;

        /* if op is one of NE, LT, LE */
        if (
            op == NE ||
            op == LT ||
            op == LE)
        {
            /*
            - NE: need to search the entire linked list of leaf indices of the B+ Tree,
            starting from the leftmost leaf index. Thus, always move to the left.

            - LT and LE: the attribute values are arranged in ascending order in the
            leaf indices of the B+ Tree. Values that satisfy these conditions, if
            any exist, will always be found in the left-most leaf index. Thus,
            always move to the left.
            */

            // load entry in the first slot of the block into intEntry
            // using IndInternal::getEntry().
            internalBlk.getEntry(&intEntry, 0);

            block = intEntry.lChild;
        }
        else
        {
            /*
            - EQ, GT and GE: move to the left child of the first entry that is
            greater than (or equal to) attrVal
            (we are trying to find the first entry that satisfies the condition.
            since the values are in ascending order we move to the left child which
            might contain more entries that satisfy the condition)
            */

            /*
             traverse through all entries of internalBlk and find an entry that
             satisfies the condition.
             if op == EQ or GE, then intEntry.attrVal >= attrVal
             if op == GT, then intEntry.attrVal > attrVal
             Hint: the helper function compareAttrs() can be used for comparing
            */
            int slot = 0, found = 0;
            while (slot < intHead.numEntries)
            {
                internalBlk.getEntry(&intEntry, slot);

                int cmpVal = compareAttrs(intEntry.attrVal, attrVal, attrCatEntry.attrType);

                if (
                    (op == EQ && cmpVal >= 0) ||
                    (op == GE && cmpVal >= 0) ||
                    (op == GT && cmpVal > 0))
                {
                    found = 1;
                    break;
                }

                slot++;
            }

            /* such an entry is found*/
            if (found)
            {
                // move to the left child of that entry
                block = intEntry.lChild; // left child of the entry
            }
            else
            {
                // move to the right child of the last entry of the block
                // i.e numEntries - 1 th entry of the block

                block = intEntry.rChild; // right child of last entry
            }
        }
    }

    // NOTE: `block` now has the block number of a leaf index block.

    /******  Identify the first leaf index entry from the current position
                that satisfies our condition (moving right)             ******/

    while (block != -1)
    {
        // load the block into leafBlk using IndLeaf::IndLeaf().
        IndLeaf leafBlk(block);
        HeadInfo leafHead;

        // load the header to leafHead using BlockBuffer::getHeader().
        leafBlk.getHeader(&leafHead);

        // declare leafEntry which will be used to store an entry from leafBlk
        Index leafEntry;

        /* while index < numEntries in leafBlk*/
        while (index < leafHead.numEntries)
        {

            // load entry corresponding to block and index into leafEntry
            // using IndLeaf::getEntry().
            leafBlk.getEntry(&leafEntry, index);

            /* comparison between leafEntry's attribute
            value and input attrVal using compareAttrs()*/
            int cmpVal = compareAttrs(leafEntry.attrVal, attrVal, attrCatEntry.attrType);

            if (
                (op == EQ && cmpVal == 0) ||
                (op == LE && cmpVal <= 0) ||
                (op == LT && cmpVal < 0) ||
                (op == GT && cmpVal > 0) ||
                (op == GE && cmpVal >= 0) ||
                (op == NE && cmpVal != 0))
            {
                // (entry satisfying the condition found)

                // set search index to {block, index}
                searchIndex = IndexId{block, index};
                AttrCacheTable::setSearchIndex(relId, attrName, &searchIndex);

                // return the recId {leafEntry.block, leafEntry.slot}.
                return RecId{leafEntry.block, leafEntry.slot};
            }
            else if ((op == EQ || op == LE || op == LT) && cmpVal > 0)
            {
                /*future entries will not satisfy EQ, LE, LT since the values
                    are arranged in ascending order in the leaves */

                // return RecId {-1, -1};
                return RecId{-1, -1};
            }

            // search next index.
            ++index;
        }

        /*only for NE operation do we have to check the entire linked list;
        for all the other op it is guaranteed that the block being searched
        will have an entry, if it exists, satisying that op. */
        if (op != NE)
        {
            break;
        }

        // block = next block in the linked list, i.e., the rblock in leafHead.
        // update index to 0.
        block = leafHead.rblock;
        index = 0;
    }

    // no entry satisying the op was found; return the recId {-1,-1}
    return RecId{-1, -1};
}

/* This method creates a B+ Tree (Indexing) for the input attribute of the specified relation. */
int BPlusTree::bPlusCreate(int relId, char attrName[ATTR_SIZE])
{

    // if relId is either RELCAT_RELID or ATTRCAT_RELID:
    //     return E_NOTPERMITTED;
    if (relId == RELCAT_RELID || relId == ATTRCAT_RELID)
    {
        return E_NOTPERMITTED;
    }

    // get the attribute catalog entry of attribute `attrName`
    // using AttrCacheTable::getAttrCatEntry()
    AttrCatEntry attrCatEntry;
    int retVal = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

    // if getAttrCatEntry fails
    //     return the error code from getAttrCatEntry
    if (retVal != SUCCESS)
    {
        return retVal;
    }

    /* if an index already exists for the attribute (check rootBlock field) */
    if (attrCatEntry.rootBlock != -1)
    {
        return SUCCESS;
    }

    /******Creating a new B+ Tree ******/

    // get a free leaf block using constructor 1 to allocate a new block
    IndLeaf rootBlockBuf;

    // (if the block could not be allocated, the appropriate error code
    //  will be stored in the blockNum member field of the object)

    // declare rootBlock to store the blockNumber of the new leaf block
    int rootBlock = rootBlockBuf.getBlockNum();

    // if there is no more disk space for creating an index
    if (rootBlock == E_DISKFULL)
    {
        return E_DISKFULL;
    }
    attrCatEntry.rootBlock = rootBlock;
    AttrCacheTable::setAttrCatEntry(relId, attrName, &attrCatEntry);

    RelCatEntry relCatEntry;

    // load the relation catalog entry into relCatEntry
    // using RelCacheTable::getRelCatEntry().
    RelCacheTable::getRelCatEntry(relId, &relCatEntry);

    /* first record block of the relation */
    int block = relCatEntry.firstBlk;

    /***** Traverse all the blocks in the relation and insert them one
           by one into the B+ Tree *****/
    while (block != -1)
    {

        // declare a RecBuffer object for `block` (using appropriate constructor)
        RecBuffer recBlk(block);

        unsigned char slotMap[relCatEntry.numSlotsPerBlk];

        // load the slot map into slotMap using RecBuffer::getSlotMap().
        recBlk.getSlotMap(slotMap);

        // for every occupied slot of the block
        for (int i = 0; i < relCatEntry.numSlotsPerBlk; i++)
        {

            if (slotMap[i] == SLOT_UNOCCUPIED)
            {
                continue;
            }

            Attribute record[relCatEntry.numAttrs];
            // load the record corresponding to the slot into `record`
            // using RecBuffer::getRecord().
            recBlk.getRecord(record, i);

            // declare recId and store the rec-id of this record in it
            // RecId recId{block, slot};
            RecId recId{block, i};

            // insert the attribute value corresponding to attrName from the record
            // into the B+ tree using bPlusInsert.
            // (note that bPlusInsert will destroy any existing bplus tree if
            // insert fails i.e when disk is full)
            retVal = bPlusInsert(relId, attrName, record[attrCatEntry.offset], recId);

            if (retVal == E_DISKFULL)
            {
                // (unable to get enough blocks to build the B+ Tree.)
                return E_DISKFULL;
            }
        }

        // get the header of the block using BlockBuffer::getHeader()
        HeadInfo head;
        recBlk.getHeader(&head);

        // set block = rblock of current block (from the header)
        block = head.rblock;
    }

    return SUCCESS;
}

/* Used to delete a B+ Tree rooted at a particular block passed as input to the method. */
int BPlusTree::bPlusDestroy(int rootBlockNum)
{
    /* if rootBlockNum lies outside the valid range [0,DISK_BLOCKS-1]*/
    if (rootBlockNum < 0 || rootBlockNum >= DISK_BLOCKS)
    {
        return E_OUTOFBOUND;
    }

    /* type of block (using StaticBuffer::getStaticBlockType())*/
    int type = StaticBuffer::getStaticBlockType(rootBlockNum);

    if (type == IND_LEAF)
    {
        // declare an instance of IndLeaf for rootBlockNum using appropriate
        // constructor
        IndLeaf indLeafBlk(rootBlockNum);

        // release the block using BlockBuffer::releaseBlock().
        indLeafBlk.releaseBlock();

        return SUCCESS;
    }
    else if (type == IND_INTERNAL)
    {
        // declare an instance of IndInternal for rootBlockNum using appropriate
        // constructor
        IndInternal indIntBlk(rootBlockNum);

        // load the header of the block using BlockBuffer::getHeader().
        HeadInfo head;
        indIntBlk.getHeader(&head);

        /*iterate through all the entries of the internalBlk and destroy the lChild
        of the first entry and rChild of all entries using BPlusTree::bPlusDestroy().
        (the rchild of an entry is the same as the lchild of the next entry.
         take care not to delete overlapping children more than once ) */
        InternalEntry entry;
        indIntBlk.getEntry(&entry, 0);
        bPlusDestroy(entry.lChild);
        for (int i = 0; i < head.numEntries; i++)
        {
            indIntBlk.getEntry(&entry, i);
            bPlusDestroy(entry.rChild);
        }

        // release the block using BlockBuffer::releaseBlock().
        indIntBlk.releaseBlock();

        return SUCCESS;
    }
    else
    {
        // (block is not an index block.)
        return E_INVALIDBLOCK;
    }
}

/* Inserts an attribute value and the rec-id of the corresponding record into a B+ tree index on the disk */
int BPlusTree::bPlusInsert(int relId, char attrName[ATTR_SIZE], Attribute attrVal, RecId recId)
{
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    AttrCatEntry attrCatEntry;
    int retVal = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

    // if getAttrCatEntry() failed
    //     return the error code
    if (retVal != SUCCESS)
    {
        return retVal;
    }

    /* rootBlock of B+ Tree (from attrCatEntry) */
    int blockNum = attrCatEntry.rootBlock;

    /* if there is no index on attribute (rootBlock is -1) */
    if (blockNum == -1)
    {
        return E_NOINDEX;
    }

    // find the leaf block to which insertion is to be done using the
    // findLeafToInsert() function

    /* findLeafToInsert(root block num, attrVal, attribute type) */
    int leafBlkNum = findLeafToInsert(blockNum, attrVal, attrCatEntry.attrType);

    // insert the attrVal and recId to the leaf block at blockNum using the
    // insertIntoLeaf() function.
    // declare a struct Index with attrVal = attrVal, block = recId.block and
    // slot = recId.slot to pass as argument to the function.
    // insertIntoLeaf(relId, attrName, leafBlkNum, Index entry)
    // NOTE: the insertIntoLeaf() function will propagate the insertion to the
    //       required internal nodes by calling the required helper functions
    //       like insertIntoInternal() or createNewRoot()
    Index leafEntry{attrVal, recId.block, recId.slot};
    retVal = insertIntoLeaf(relId, attrName, leafBlkNum, leafEntry);

    /* if insertIntoLeaf() returns E_DISKFULL */
    if (retVal == E_DISKFULL)
    {
        // destroy the existing B+ tree by passing the rootBlock to bPlusDestroy().
        bPlusDestroy(blockNum);

        // update the rootBlock of attribute catalog cache entry to -1 using
        // AttrCacheTable::setAttrCatEntry().
        attrCatEntry.rootBlock = -1;
        AttrCacheTable::setAttrCatEntry(relId, attrName, &attrCatEntry);

        return E_DISKFULL;
    }

    return SUCCESS;
}

/* Used to find the leaf index block to which an attribute would be inserted to in the B+ insertion process. */
int BPlusTree::findLeafToInsert(int rootBlock, Attribute attrVal, int attrType)
{
    int blockNum = rootBlock;

    /* while block is not of type IND_LEAF */
    while (StaticBuffer::getStaticBlockType(blockNum) != IND_LEAF)
    { // use StaticBuffer::getStaticBlockType()

        // declare an IndInternal object for block using appropriate constructor
        IndInternal indIntBlk(blockNum);

        // get header of the block using BlockBuffer::getHeader()
        HeadInfo head;
        indIntBlk.getHeader(&head);

        /* iterate through all the entries, to find the first entry whose
             attribute value >= value to be inserted.
             NOTE: the helper function compareAttrs() declared in BlockBuffer.h
                   can be used to compare two Attribute values. */
        int found = 0;
        InternalEntry entry;
        for (int i = 0; i < head.numEntries; i++)
        {
            indIntBlk.getEntry(&entry, i);

            int cmpVal = compareAttrs(entry.attrVal, attrVal, attrType);

            if (cmpVal >= 0)
            {
                found = 1;
                break;
            }
        }

        /* if no such entry is found*/
        if (!found)
        {
            // set blockNum = rChild of (nEntries-1)'th entry of the block
            // (i.e. rightmost child of the block)
            blockNum = entry.rChild;
        }
        else
        {
            // set blockNum = lChild of the entry that was found
            blockNum = entry.lChild;
        }
    }

    return blockNum;
}

/* Used to insert an index entry into a leaf index block of an existing B+ tree. */
int BPlusTree::insertIntoLeaf(int relId, char attrName[ATTR_SIZE], int blockNum, Index indexEntry)
{
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    AttrCatEntry attrCatEntry;
    AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

    // declare an IndLeaf instance for the block using appropriate constructor
    IndLeaf indLeafBlk(blockNum);

    HeadInfo blockHeader;
    // store the header of the leaf index block into blockHeader
    // using BlockBuffer::getHeader()
    indLeafBlk.getHeader(&blockHeader);

    // the following variable will be used to store a list of index entries with
    // existing indices + the new index to insert
    Index indices[blockHeader.numEntries + 1];

    /*
    Iterate through all the entries in the block and copy them to the array indices.
    Also insert `indexEntry` at appropriate position in the indices array maintaining
    the ascending order.
    - use IndLeaf::getEntry() to get the entry
    - use compareAttrs() declared in BlockBuffer.h to compare two Attribute structs
    */
    for (int i = 0; i < blockHeader.numEntries; i++)
    {
        indLeafBlk.getEntry(&indices[i], i);
    }

    int i, retVal;
    for (i = 0; i < blockHeader.numEntries; i++)
    {

        retVal = compareAttrs(indices[i].attrVal, indexEntry.attrVal, attrCatEntry.attrType);
        if (retVal > 0)
        {
            // shift all entries from i to numEntries-1 to the right by one position
            for (int j = blockHeader.numEntries; j > i; j--)
            {
                indices[j] = indices[j - 1];
            }

            // insert indexEntry at index i
            indices[i] = indexEntry;
            break;
        }
    }
    if (i == blockHeader.numEntries)
    {
        // insert indexEntry at index i
        indices[i] = indexEntry;
    }

    if (blockHeader.numEntries != MAX_KEYS_LEAF)
    {
        // (leaf block has not reached max limit)

        // increment blockHeader.numEntries and update the header of block
        // using BlockBuffer::setHeader().
        blockHeader.numEntries++;
        indLeafBlk.setHeader(&blockHeader);

        // iterate through all the entries of the array `indices` and populate the
        // entries of block with them using IndLeaf::setEntry().
        for (int i = 0; i < blockHeader.numEntries; i++)
        {
            indLeafBlk.setEntry(&indices[i], i);
        }

        return SUCCESS;
    }

    // If we reached here, the `indices` array has more than entries than can fit
    // in a single leaf index block. Therefore, we will need to split the entries
    // in `indices` between two leaf blocks. We do this using the splitLeaf() function.
    // This function will return the blockNum of the newly allocated block or
    // E_DISKFULL if there are no more blocks to be allocated.

    int newRightBlk = splitLeaf(blockNum, indices);

    // if splitLeaf() returned E_DISKFULL
    //     return E_DISKFULL
    if (newRightBlk == E_DISKFULL)
    {
        return E_DISKFULL;
    }

    /* if the current leaf block was not the root */
    if (blockHeader.pblock != -1)
    { // check pblock in header
        // insert the middle value from `indices` into the parent block using the
        // insertIntoInternal() function. (i.e the last value of the left block)
        // the middle value will be at index 31 (given by constant MIDDLE_INDEX_LEAF)

        // create a struct InternalEntry with attrVal = indices[MIDDLE_INDEX_LEAF].attrVal,
        // lChild = currentBlock, rChild = newRightBlk and pass it as argument to
        // the insertIntoInternalFunction as follows
        InternalEntry newIntEntry;
        newIntEntry.attrVal = indices[MIDDLE_INDEX_LEAF].attrVal;
        newIntEntry.lChild = blockNum;
        newIntEntry.rChild = newRightBlk;

        // insertIntoInternal(relId, attrName, parent of current block, new internal entry)
        return insertIntoInternal(relId, attrName, blockHeader.pblock, newIntEntry);
    }
    else
    {
        // the current block was the root block and is now split. a new internal index
        // block needs to be allocated and made the root of the tree.
        // To do this, call the createNewRoot() function with the following arguments

        // createNewRoot(relId, attrName, indices[MIDDLE_INDEX_LEAF].attrVal,
        //               current block, new right block)
        return createNewRoot(relId, attrName, indices[MIDDLE_INDEX_LEAF].attrVal, blockNum, newRightBlk);
    }

    // if either of the above calls returned an error (E_DISKFULL), then return that
    // else return SUCCESS
}

/* Distributes an array of index entries between an existing leaf index block and a newly allocated leaf index block. */
int BPlusTree::splitLeaf(int leafBlockNum, Index indices[])
{
    // declare rightBlk, an instance of IndLeaf using constructor 1 to obtain new
    // leaf index block that will be used as the right block in the splitting
    IndLeaf rightBlk;

    // declare leftBlk, an instance of IndLeaf using constructor 2 to read from
    // the existing leaf block
    IndLeaf leftBlk(leafBlockNum);

    /* block num of right blk */
    int rightBlkNum = rightBlk.getBlockNum();
    /* block num of left blk */
    int leftBlkNum = leafBlockNum;

    /* if newly allocated block has blockNum E_DISKFULL */
    if (rightBlkNum == E_DISKFULL)
    {
        //(failed to obtain a new leaf index block because the disk is full)
        return E_DISKFULL;
    }

    HeadInfo leftBlkHeader, rightBlkHeader;
    // get the headers of left block and right block using BlockBuffer::getHeader()
    leftBlk.getHeader(&leftBlkHeader);
    rightBlk.getHeader(&rightBlkHeader);

    // set rightBlkHeader with the following values
    // - number of entries = (MAX_KEYS_LEAF+1)/2 = 32,
    // - pblock = pblock of leftBlk
    // - lblock = leftBlkNum
    // - rblock = rblock of leftBlk
    // and update the header of rightBlk using BlockBuffer::setHeader()
    rightBlkHeader.numEntries = (MAX_KEYS_LEAF + 1) / 2;
    rightBlkHeader.pblock = leftBlkHeader.pblock;
    rightBlkHeader.lblock = leftBlkNum;
    rightBlkHeader.rblock = leftBlkHeader.rblock;
    rightBlk.setHeader(&rightBlkHeader);

    // set leftBlkHeader with the following values
    // - number of entries = (MAX_KEYS_LEAF+1)/2 = 32
    // - rblock = rightBlkNum
    // and update the header of leftBlk using BlockBuffer::setHeader() */
    leftBlkHeader.numEntries = (MAX_KEYS_LEAF + 1) / 2;
    leftBlkHeader.rblock = rightBlkNum;
    leftBlk.setHeader(&leftBlkHeader);

    // set the first 32 entries of leftBlk = the first 32 entries of indices array
    // and set the first 32 entries of newRightBlk = the next 32 entries of
    // indices array using IndLeaf::setEntry().
    for (int i = 0; i < leftBlkHeader.numEntries; i++)
    {
        leftBlk.setEntry(&indices[i], i);
    }
    for (int i = 0; i < rightBlkHeader.numEntries; i++)
    {
        rightBlk.setEntry(&indices[i + leftBlkHeader.numEntries], i);
    }

    return rightBlkNum;
}

/* Used to insert an index entry into an internal index block of an existing B+ tree. This
function will call itself to handle any updation required to it's parent internal index blocks. */
int BPlusTree::insertIntoInternal(int relId, char attrName[ATTR_SIZE], int intBlockNum, InternalEntry intEntry)
{
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    AttrCatEntry attrCatEntry;
    AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

    // declare intBlk, an instance of IndInternal using constructor 2 for the block
    // corresponding to intBlockNum
    IndInternal intBlk(intBlockNum);

    HeadInfo blockHeader;
    // load blockHeader with header of intBlk using BlockBuffer::getHeader().
    intBlk.getHeader(&blockHeader);

    // declare internalEntries to store all existing entries + the new entry
    InternalEntry internalEntries[blockHeader.numEntries + 1];

    /*
    Iterate through all the entries in the block and copy them to the array
    `internalEntries`. Insert `indexEntry` at appropriate position in the
    array maintaining the ascending order.
        - use IndInternal::getEntry() to get the entry
        - use compareAttrs() to compare two structs of type Attribute

    Update the lChild of the internalEntry immediately following the newly added
    entry to the rChild of the newly added entry.
    */
    int i, retVal;
    for (i = 0; i < blockHeader.numEntries; i++)
    {
        intBlk.getEntry(&internalEntries[i], i);
    }

    for (i = 0; i < blockHeader.numEntries; i++)
    {
        retVal = compareAttrs(internalEntries[i].attrVal, intEntry.attrVal, attrCatEntry.attrType);
        if (retVal > 0)
        {
            // shift all entries from i to numEntries-1 to the right by one position
            for (int j = blockHeader.numEntries; j > i; j--)
            {
                internalEntries[j] = internalEntries[j - 1];
            }

            // insert intEntry at index i
            internalEntries[i] = intEntry;
            break;
        }
    }
    if (i == blockHeader.numEntries)
    {
        // insert intEntry at index i
        internalEntries[i] = intEntry;
    }

    // updating lChild of the entry immediately following the newly added entry
    if (i < blockHeader.numEntries)
    {
        internalEntries[i + 1].lChild = intEntry.rChild;
    }
    // updating rChild of the entry immediately preceding the newly added entry
    if (i > 0)
    {
        internalEntries[i - 1].rChild = intEntry.lChild;
    }

    if (blockHeader.numEntries != MAX_KEYS_INTERNAL)
    {
        // (internal index block has not reached max limit)

        // increment blockheader.numEntries and update the header of intBlk
        // using BlockBuffer::setHeader().
        blockHeader.numEntries++;
        intBlk.setHeader(&blockHeader);

        // iterate through all entries in internalEntries array and populate the
        // entries of intBlk with them using IndInternal::setEntry().
        for (i = 0; i < blockHeader.numEntries; i++)
        {
            intBlk.setEntry(&internalEntries[i], i);
        }

        return SUCCESS;
    }

    // If we reached here, the `internalEntries` array has more than entries than
    // can fit in a single internal index block. Therefore, we will need to split
    // the entries in `internalEntries` between two internal index blocks. We do
    // this using the splitInternal() function.
    // This function will return the blockNum of the newly allocated block or
    // E_DISKFULL if there are no more blocks to be allocated.

    int newRightBlk = splitInternal(intBlockNum, internalEntries);

    /* if splitInternal() returned E_DISKFULL */
    if (newRightBlk == E_DISKFULL)
    {

        // Using bPlusDestroy(), destroy the right subtree, rooted at intEntry.rChild.
        // This corresponds to the tree built up till now that has not yet been
        // connected to the existing B+ Tree
        bPlusDestroy(intEntry.rChild);

        return E_DISKFULL;
    }

    /* if the current block was not the root */
    if (blockHeader.pblock != -1)
    { // (check pblock in header)
        // insert the middle value from `internalEntries` into the parent block
        // using the insertIntoInternal() function (recursively).

        // the middle value will be at index 50 (given by constant MIDDLE_INDEX_INTERNAL)

        // create a struct InternalEntry with lChild = current block, rChild = newRightBlk
        // and attrVal = internalEntries[MIDDLE_INDEX_INTERNAL].attrVal
        // and pass it as argument to the insertIntoInternalFunction as follows
        InternalEntry newIntEntry;

        newIntEntry.lChild = intBlockNum;
        newIntEntry.attrVal = internalEntries[MIDDLE_INDEX_INTERNAL].attrVal;
        newIntEntry.rChild = newRightBlk;

        // insertIntoInternal(relId, attrName, parent of current block, new internal entry)
        return insertIntoInternal(relId, attrName, blockHeader.pblock, newIntEntry);
    }
    else
    {
        // the current block was the root block and is now split. a new internal index
        // block needs to be allocated and made the root of the tree.
        // To do this, call the createNewRoot() function with the following arguments

        // createNewRoot(relId, attrName,
        //               internalEntries[MIDDLE_INDEX_INTERNAL].attrVal,
        //               current block, new right block)
        return createNewRoot(relId, attrName, internalEntries[MIDDLE_INDEX_INTERNAL].attrVal, intBlockNum, newRightBlk);
    }

    // if either of the above calls returned an error (E_DISKFULL), then return that
    // else return SUCCESS
}

/* Distributes an array of index entries between an existing
internal index block and a newly allocated internal index block. */
int BPlusTree::splitInternal(int intBlockNum, InternalEntry internalEntries[])
{
    // declare rightBlk, an instance of IndInternal using constructor 1 to obtain new
    // internal index block that will be used as the right block in the splitting
    IndInternal rightBlk;

    // declare leftBlk, an instance of IndInternal using constructor 2 to read from
    // the existing internal index block
    IndInternal leftBlk(intBlockNum);

    /* block num of right blk */
    int rightBlkNum = rightBlk.getBlockNum();
    /* block num of left blk */
    int leftBlkNum = intBlockNum;

    /* newly allocated block has blockNum E_DISKFULL */
    if (rightBlkNum == E_DISKFULL)
    {
        //(failed to obtain a new internal index block because the disk is full)
        return E_DISKFULL;
    }

    HeadInfo leftBlkHeader, rightBlkHeader;
    // get the headers of left block and right block using BlockBuffer::getHeader()
    leftBlk.getHeader(&leftBlkHeader);
    rightBlk.getHeader(&rightBlkHeader);

    // set rightBlkHeader with the following values
    // - number of entries = (MAX_KEYS_INTERNAL)/2 = 50
    // - pblock = pblock of leftBlk
    // and update the header of rightBlk using BlockBuffer::setHeader()
    rightBlkHeader.numEntries = (MAX_KEYS_INTERNAL) / 2;
    rightBlkHeader.pblock = leftBlkHeader.pblock;
    rightBlk.setHeader(&rightBlkHeader);

    // set leftBlkHeader with the following values
    // - number of entries = (MAX_KEYS_INTERNAL)/2 = 50
    // - rblock = rightBlkNum
    // and update the header using BlockBuffer::setHeader()
    leftBlkHeader.numEntries = (MAX_KEYS_INTERNAL) / 2;
    leftBlkHeader.rblock = rightBlkNum;
    leftBlk.setHeader(&leftBlkHeader);

    /*
    - set the first 50 entries of leftBlk = index 0 to 49 of internalEntries
      array
    - set the first 50 entries of newRightBlk = entries from index 51 to 100
      of internalEntries array using IndInternal::setEntry().
      (index 50 will be moving to the parent internal index block)
    */
    for (int i = 0; i < leftBlkHeader.numEntries; i++)
    {
        leftBlk.setEntry(&internalEntries[i], i);
    }
    for (int i = 0; i < rightBlkHeader.numEntries; i++)
    {
        rightBlk.setEntry(&internalEntries[i + MIDDLE_INDEX_INTERNAL + 1], i);
    }

    /* block type of a child of any entry of the internalEntries array */
    //            (use StaticBuffer::getStaticBlockType())
    int type = StaticBuffer::getStaticBlockType(internalEntries[0].lChild);

    /* for each child block of the new right block */
    BlockBuffer childBlk(internalEntries[MIDDLE_INDEX_INTERNAL + 1].lChild);

    HeadInfo childHeader;
    childBlk.getHeader(&childHeader);
    childHeader.pblock = rightBlkNum;
    childBlk.setHeader(&childHeader);

    for (int i = 0; i < MIDDLE_INDEX_INTERNAL; i++)
    {
        // declare an instance of BlockBuffer to access the child block using
        // constructor 2
        BlockBuffer childBlk(internalEntries[i + MIDDLE_INDEX_INTERNAL + 1].rChild);

        // update pblock of the block to rightBlkNum using BlockBuffer::getHeader()
        // and BlockBuffer::setHeader().
        HeadInfo childHeader;
        childBlk.getHeader(&childHeader);
        childHeader.pblock = rightBlkNum;
        childBlk.setHeader(&childHeader);
    }

    return rightBlkNum;
}

/* This function will allocate a new root block and update the attribute cache
entry of the attribute in the specified relation to point to the new root block. */
int BPlusTree::createNewRoot(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int lChild, int rChild)
{
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    AttrCatEntry attrCatEntry;
    AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

    // declare newRootBlk, an instance of IndInternal using appropriate constructor
    // to allocate a new internal index block on the disk
    IndInternal newRootBlk;

    /* block number of newRootBlk */;
    int newRootBlkNum = newRootBlk.getBlockNum();

    if (newRootBlkNum == E_DISKFULL)
    {
        // (failed to obtain an empty internal index block because the disk is full)

        // Using bPlusDestroy(), destroy the right subtree, rooted at rChild.
        // This corresponds to the tree built up till now that has not yet been
        // connected to the existing B+ Tree
        bPlusDestroy(rChild);

        return E_DISKFULL;
    }

    // update the header of the new block with numEntries = 1 using
    // BlockBuffer::getHeader() and BlockBuffer::setHeader()
    HeadInfo newRootBlkHeader;
    newRootBlk.getHeader(&newRootBlkHeader);
    newRootBlkHeader.numEntries = 1;
    newRootBlk.setHeader(&newRootBlkHeader);

    // create a struct InternalEntry with lChild, attrVal and rChild from the
    // arguments and set it as the first entry in newRootBlk using IndInternal::setEntry()
    InternalEntry newRootEntry;
    newRootEntry.lChild = lChild;
    newRootEntry.attrVal = attrVal;
    newRootEntry.rChild = rChild;
    newRootBlk.setEntry(&newRootEntry, 0);

    // declare BlockBuffer instances for the `lChild` and `rChild` blocks using
    // appropriate constructor and update the pblock of those blocks to `newRootBlkNum`
    // using BlockBuffer::getHeader() and BlockBuffer::setHeader()
    RecBuffer lChildBlk(lChild);
    RecBuffer rChildBlk(rChild);

    HeadInfo lChildHeader, rChildHeader;
    lChildBlk.getHeader(&lChildHeader);
    rChildBlk.getHeader(&rChildHeader);

    lChildHeader.pblock = newRootBlkNum;
    rChildHeader.pblock = newRootBlkNum;

    lChildBlk.setHeader(&lChildHeader);
    rChildBlk.setHeader(&rChildHeader);

    // update rootBlock = newRootBlkNum for the entry corresponding to `attrName`
    // in the attribute cache using AttrCacheTable::setAttrCatEntry().
    attrCatEntry.rootBlock = newRootBlkNum;
    AttrCacheTable::setAttrCatEntry(relId, attrName, &attrCatEntry);

    return SUCCESS;
}