#include "OpenRelTable.h"

#include <cstring>
#include <cstdlib>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

/*
  * Initializes the meta information of each entry of
    the Open Relation Table to initial empty conditions.
  * It also loads the entries of the Relation Catalog relation
    and Attribute Catalog relation to the Relation Cache Table
    and Attribute Cache Table.
*/
OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
    tableMetaInfo[i].free = true;
  }

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Relation Cache Table ****/
  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/

  // set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

  RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = relCacheEntry;

  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Attribute Cache Table ****/
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);

  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

  // iterate through all the attributes of the relation catalog and create a linked
  // list of AttrCacheEntry (slots 0 to 5)
  // for each of the entries, set
  //    attrCacheEntry.recId.block = ATTRCAT_BLOCK;
  //    attrCacheEntry.recId.slot = i   (0 to 5)
  //    and attrCacheEntry.next appropriately
  // NOTE: allocate each entry dynamically using malloc
  AttrCacheEntry * head , * curr;
  head = curr = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
  int i;
  for (i = 0; i < RELCAT_NO_ATTRS - 1; i++) {
    attrCatBlock.getRecord(attrCatRecord, i);
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &curr->attrCatEntry);
    curr->recId.slot = i;
    curr->recId.block = ATTRCAT_BLOCK;
    curr->next = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    curr = curr->next;
  }
  attrCatBlock.getRecord(attrCatRecord, i);
  AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &curr->attrCatEntry);
  curr->next = nullptr;
  curr->recId.slot = i;
  curr->recId.block = ATTRCAT_BLOCK;
  i++;

  // set the next field in the last entry to nullptr

  AttrCacheTable::attrCache[RELCAT_RELID] = head;

  /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/

  // set up the attributes of the attribute cache similarly.
  // read slots 6-11 from attrCatBlock and initialise recId appropriately
  head = curr = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
  for (; i < RELCAT_NO_ATTRS + ATTRCAT_NO_ATTRS - 1; i++) {
    attrCatBlock.getRecord(attrCatRecord, i);
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &curr->attrCatEntry);
    curr->recId.slot = i;
    curr->recId.block = ATTRCAT_BLOCK;
    curr->next = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    curr = curr->next;
  }
  attrCatBlock.getRecord(attrCatRecord, i);
  AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &curr->attrCatEntry);
  curr->next = nullptr;
  curr->recId.slot = i;
  curr->recId.block = ATTRCAT_BLOCK;
  i++;

  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]
  AttrCacheTable::attrCache[ATTRCAT_RELID] = head;

  /************ Setting up tableMetaInfo entries ************/

  // in the tableMetaInfo array
  //   set free = false for RELCAT_RELID and ATTRCAT_RELID
  //   set relname for RELCAT_RELID and ATTRCAT_RELID

  // for relcat relid
  tableMetaInfo[RELCAT_RELID].free = false;
  strcpy(tableMetaInfo[RELCAT_RELID].relName, RELCAT_RELNAME);

  // for attrcat relid
  tableMetaInfo[ATTRCAT_RELID].free = false;
  strcpy(tableMetaInfo[ATTRCAT_RELID].relName, ATTRCAT_RELNAME);
}

/* Returns `index` of an unoccupied entry in the Open Relation Table. */
int OpenRelTable::getFreeOpenRelTableEntry() {

  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/
  for (int i = ATTRCAT_RELID + 1; i < MAX_OPEN; i++) {
    if (tableMetaInfo[i].free == true)
    {
      return i;
    }

  }

  // if found return the relation id, else return E_CACHEFULL.
  return E_CACHEFULL;
}

/* Returns the relation id, that is, the index, of the entry
corresponding to the input relation in the Open Relation Table. */
int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

  /* traverse through the tableMetaInfo array,
    find the entry in the Open Relation Table corresponding to relName.*/
  for (int i = 0; i < MAX_OPEN; i++)
  {
    if (!tableMetaInfo[i].free && strcmp(tableMetaInfo[i].relName, relName) == 0) {
      return i;
    }
  }

  // if found return the relation id, else indicate that the relation do not
  // have an entry in the Open Relation Table.
  return E_RELNOTOPEN;
}


int OpenRelTable::openRel(char relName[ATTR_SIZE]) {

  int ret = getRelId(relName);
  if(ret >= 0){
    // (checked using OpenRelTable::getRelId())

    // return that relation id;
    return ret;
  }

  /* find a free slot in the Open Relation Table
     using OpenRelTable::getFreeOpenRelTableEntry(). */
  ret = getFreeOpenRelTableEntry();

  if (ret == E_CACHEFULL){
    return E_CACHEFULL;
  }

  // let relId be used to store the free slot.
  int relId = ret;

  /****** Setting up Relation Cache entry for the relation ******/

  /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/

  RelCatEntry relCatEntry;
  RelCacheTable::getRelCatEntry(RELCAT_RELID, &relCatEntry);

  AttrCatEntry attrCatEntry;
  AttrCacheTable::getAttrCatEntry(RELCAT_RELID, RELCAT_REL_NAME_INDEX, &attrCatEntry);
  
  RelCacheTable::resetSearchIndex(RELCAT_RELID);
  Attribute attrVal;
  strcpy(attrVal.sVal, relName);

  // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
  RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, attrCatEntry.attrName, attrVal, EQ);

  /* relcatRecId == {-1, -1} */
  if (relcatRecId.block == -1 && relcatRecId.slot == -1) {
    // (the relation is not found in the Relation Catalog.)
    return E_RELNOTEXIST;
  }

  /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      update the recId field of this Relation Cache entry to relcatRecId.
      use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  */
  RecBuffer recBlock(relcatRecId.block);
  Attribute record[RELCAT_NO_ATTRS];
  recBlock.getRecord(record, relcatRecId.slot);
  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(record, &relCacheEntry.relCatEntry);
  relCacheEntry.recId = relcatRecId;

  RelCacheTable::relCache[relId] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[relId]) = relCacheEntry;

  /****** Setting up Attribute Cache entry for the relation ******/

  // let listHead be used to hold the head of the linked list of attrCache entries.
  AttrCacheEntry* listHead, *curr;
  listHead = curr = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));

  int numAttrs = relCacheEntry.relCatEntry.numAttrs;

  for (int i = 0; i < numAttrs - 1; i++) {
    curr->next = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    curr = curr->next;
  }
  curr->next = nullptr;

  RelCacheTable::getRelCatEntry(ATTRCAT_RELID, &relCatEntry);
  AttrCacheTable::getAttrCatEntry(ATTRCAT_RELID, ATTRCAT_REL_NAME_INDEX, &attrCatEntry);
  strcpy(attrVal.sVal, relName);
  char attrName[ATTR_SIZE];
  strcpy(attrName, ATTRCAT_ATTR_RELNAME);

  /*iterate over all the entries in the Attribute Catalog corresponding to each
  attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
  care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
  corresponding to Attribute Catalog before the first call to linearSearch().*/

  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
  curr = listHead;

  for (int i = 0; i < numAttrs; i++)
  {
      /* let attrcatRecId store a valid record id an entry of the relation, relName,
      in the Attribute Catalog.*/
      RecId attrcatRecId;
      attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, attrName, attrVal, EQ);

      /* read the record entry corresponding to attrcatRecId and create an
      Attribute Cache entry on it using RecBuffer::getRecord() and
      AttrCacheTable::recordToAttrCatEntry().
      update the recId field of this Attribute Cache entry to attrcatRecId.
      add the Attribute Cache entry to the linked list of listHead .*/
      // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
      RecBuffer recBlock(attrcatRecId.block);
      Attribute record[ATTRCAT_NO_ATTRS];
      recBlock.getRecord(record, attrcatRecId.slot);
      AttrCacheTable::recordToAttrCatEntry(record, &curr->attrCatEntry);
      curr->recId = attrcatRecId; 

      curr = curr->next;
  }

  // set the relIdth entry of the AttrCacheTable to listHead.
  AttrCacheTable::attrCache[relId] = listHead;

  /****** Setting up metadata in the Open Relation Table for the relation******/

  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.
  tableMetaInfo[relId].free = false;
  strcpy(tableMetaInfo[relId].relName, relName);

  return relId;
}


int OpenRelTable::closeRel(int relId) {
  /* rel-id corresponds to relation catalog or attribute catalog*/
  if (relId == ATTRCAT_RELID || relId == RELCAT_RELID) {
    return E_NOTPERMITTED;
  }

  /* 0 <= relId < MAX_OPEN */
  if (relId >= MAX_OPEN || relId < 0) {
    return E_OUTOFBOUND;
  }

  /* rel-id corresponds to a free slot*/
  if (tableMetaInfo[relId].free) {
    return E_RELNOTOPEN;
  }

  // free the memory allocated in the relation and attribute caches which was
  // allocated in the OpenRelTable::openRel() function
  free(RelCacheTable::relCache[relId]);
  AttrCacheEntry *entry = AttrCacheTable::attrCache[relId], *temp = nullptr;
  while (entry) {
    temp = entry;
    entry = entry->next;
    free(temp);
  }

  // update `tableMetaInfo` to set `relId` as a free slot
  // update `relCache` and `attrCache` to set the entry at `relId` to nullptr
  tableMetaInfo[relId].free = true;
  RelCacheTable::relCache[relId] = nullptr;
  AttrCacheTable::attrCache[relId] = nullptr;

  return SUCCESS;
}


OpenRelTable::~OpenRelTable() {

  // close all open relations (from rel-id = 2 onwards. Why?)
  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      OpenRelTable::closeRel(i); // we will implement this function later
    }
  }

  // free the memory allocated for rel-id 0 and 1 in the caches
  for (int i = 0; i < ATTRCAT_RELID + 1; i++) {
    // free relcache
    free(RelCacheTable::relCache[i]);

    // free attrcache
    AttrCacheEntry* entry = AttrCacheTable::attrCache[i], * temp;
    while (entry) {
      temp = entry;
      entry = entry->next;
      free(temp);
    }
  }
}