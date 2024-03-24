#include "BPlusTree.h"

#include <cstring>
#include <iostream>
RecId BPlusTree::bPlusSearch(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int op)
{
    // declare searchIndex which will be used to store search index for attrName.
    IndexId searchIndex;
    // int count = 0;
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
                // count++;
                if (
                    (op == EQ && cmpVal == 0) ||
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
            // count++;
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
                // printf("%d ", count);
                //  return the recId {leafEntry.block, leafEntry.slot}.
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
    int ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);
    // if getAttrCatEntry fails
    //     return the error code from getAttrCatEntry
    if (ret != SUCCESS)
    {
        return ret;
    }
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

    RelCatEntry relCatEntry;

    // load the relation catalog entry into relCatEntry
    // using RelCacheTable::getRelCatEntry().
    RelCacheTable::getRelCatEntry(relId, &relCatEntry);
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
        for (int i = 0; slotMap[i] == SLOT_OCCUPIED; i++)
        {
            Attribute record[relCatEntry.numAttrs];
            // load the record corresponding to the slot into `record`
            // using RecBuffer::getRecord().
            recBlk.getRecord(&record, i);
            // declare recId and store the rec-id of this record in it
            // RecId recId{block, slot};
            RecId recId = {block, i};
            // insert the attribute value corresponding to attrName from the record
            // into the B+ tree using bPlusInsert.
            // (note that bPlusInsert will destroy any existing bplus tree if
            // insert fails i.e when disk is full)
            // retVal = bPlusInsert(relId, attrName, attribute value, recId);
            ret = bPlusInsert(relId, attrName, record[i], recId);
            if (ret == E_DISKFULL)
            {
                // (unable to get enough blocks to build the B+ Tree.)
                return E_DISKFULL;
            }
        }

        // get the header of the block using BlockBuffer::getHeader()
        HeadInfo head;
        recBlk.getheader(&head);
        // set block = rblock of current block (from the header)
        block = head.rblock;
    }

    return SUCCESS;
}

int BPlusTree::bPlusDestroy(int rootBlockNum)
{
    if (rootBlockNum < 0 || rootBlockNum >= DISK_BLOCKS)
    {
        return E_OUTOFBOUND;
    }

    int type = StaticBuffer::getStaticBlockType(rootBlockNum);

    if (type == IND_LEAF)
    {
        // declare an instance of IndLeaf for rootBlockNum using appropriate
        // constructor
        IndLeaf leaf(rootBlockNum);
        // release the block using BlockBuffer::releaseBlock().
        ind.releaseBlock();
        return SUCCESS;
    }
    else if (type == IND_INTERNAL)
    {
        // declare an instance of IndInternal for rootBlockNum using appropriate
        // constructor
        IndInternal internal(rootBlockNum);
        // load the header of the block using BlockBuffer::getHeader().
        HeadInfo head;
        internal.getHeader(&head);
        /*iterate through all the entries of the internalBlk and destroy the lChild
        of the first entry and rChild of all entries using BPlusTree::bPlusDestroy().
        (the rchild of an entry is the same as the lchild of the next entry.
         take care not to delete overlapping children more than once ) */
        for (int i = 0; i < 100; i++)
        {
            InternalEntry Internal;
            internal.getEntry(&Internal, i);
            if (i == 0)
            {
                BPlusTree::bPlusDestroy(Internal.lChild);
            }
            BPlusTree::bPlusDestroy(Internal.rChild);
        }
        // release the block using BlockBuffer::releaseBlock().
        internal.releaseBlock();
        return SUCCESS;
    }
    else
    {
        // (block is not an index block.)
        return E_INVALIDBLOCK;
    }
}

int BPlusTree::bPlusInsert(int relId, char attrName[ATTR_SIZE], Attribute attrVal, RecId recId)
{
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    AttrCatEntry attrCatEntry;
    int ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);
    // if getAttrCatEntry() failed
    //     return the error code
    if (ret != SUCCESS)
    {
        return ret;
    }
    int blockNum = attrCatEntry.rootBlock;

    if (blockNum == -1)
    {
        return E_NOINDEX;
    }

    // find the leaf block to which insertion is to be done using the
    // findLeafToInsert() function

    int leafBlkNum = findLeafToInsert(blockNum, attrVal, attrCatEntry.attrType);

    // insert the attrVal and recId to the leaf block at blockNum using the
    // insertIntoLeaf() function.
    insertIntoLeaf(relId, attrName, leafBlkNum, )
        // declare a struct Index with attrVal = attrVal, block = recId.block and
        // slot = recId.slot to pass as argument to the function.
        // insertIntoLeaf(relId, attrName, leafBlkNum, Index entry)
        // NOTE: the insertIntoLeaf() function will propagate the insertion to the
        //       required internal nodes by calling the required helper functions
        //       like insertIntoInternal() or createNewRoot()

        if (/*insertIntoLeaf() returns E_DISKFULL */)
    {
        // destroy the existing B+ tree by passing the rootBlock to bPlusDestroy().

        // update the rootBlock of attribute catalog cache entry to -1 using
        // AttrCacheTable::setAttrCatEntry().

        return E_DISKFULL;
    }

    return SUCCESS;
}