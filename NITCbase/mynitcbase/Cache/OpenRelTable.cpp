#include "OpenRelTable.h"

#include <cstring>
#include <cstdlib>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
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
    
    tableMetaInfo[RELCAT_RELID].free = false;
    tableMetaInfo[RELCAT_RELID].relName = RELCAT_RELNAME;

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

  /**** setting up Student Catalog relation in the Relation Cache Table ****/

  relCatBlock.getRecord(relCatRecord, ATTRCAT_RELID + 1);

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID+1]
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = ATTRCAT_RELID + 1;

  RelCacheTable::relCache[ATTRCAT_RELID + 1] = (struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID + 1]) = relCacheEntry;

   tableMetaInfo[ATTRCAT_RELID].free = false;
   tableMetaInfo[ATTRCAT_RELID].relName = ATTRCAT_RELNAME;

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
  curr->recId.slot = RELCAT_NO_ATTRS - 1;
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

  /**** setting up Student Catalog relation in the Attribute Cache Table ****/
  i = 12;
  head = curr = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
  for (; i < RELCAT_NO_ATTRS + ATTRCAT_NO_ATTRS + relCacheEntry.relCatEntry.numAttrs - 1; i++)
  {
      attrCatBlock.getRecord(attrCatRecord, i);
      AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &curr->attrCatEntry);
      curr->recId.slot = i;
      curr->recId.block = ATTRCAT_BLOCK;
      curr->next = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
      curr = curr->next;
  }
  attrCatBlock.getRecord(attrCatRecord, i);
  AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &curr->attrCatEntry);
  curr->next = nullptr;
  curr->recId.slot = i;
  curr->recId.block = ATTRCAT_BLOCK;
  i++;

  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID+1]
  AttrCacheTable::attrCache[ATTRCAT_RELID + 1] = head;
  
}

OpenRelTable::~OpenRelTable() {

  // close all open relations (from rel-id = 2 onwards. Why?)
  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      OpenRelTable::closeRel(i); // we will implement this function later
    }
  }

  // free the memory allocated for rel-id 0 and 1 in the caches
  for( int i=0; i< ATTRCAT_RELID+1 ; i++){
    free(OpenRelTable:: relCache[i]);
  
  AttrCacheEntry * entry = AttrCacheTable::attrCache[i] , *temp;
  while(entry! = nullptr){
    temp = entry ; 
    entry = entry->next;
    free(temp);
  }
  }
}

/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the relations
and open the appropriate one.
*/
int OpenRelTable::getRelId(unsigned char relName[ATTR_SIZE]) {

  /* traverse through the tableMetaInfo array,
    find the entry in the Open Relation Table corresponding to relName.*/
for(int i=0;i< MAX_OPEN ; i++){
  if(strcmp(tableMetaInfo[i].relName , relName) == 0){
    return i;
  }
}
  // if found return the relation id, else indicate that the relation do not
  // have an entry in the Open Relation Table.
  return E_RELNOTOPEN; 
}

int OpenRelTable::getFreeOpenRelTableEntry() {

  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/
for(int i = 0; i < MAX_OPEN; i++){
  if(tableMetaInfo[i].free = true){
    return i;
  }
}
  // if found return the relation id, else return E_CACHEFULL.
  return E_CACHEFULL;
}

int OpenRelTable::openRel(unsigned char relName[ATTR_SIZE]) {

  /* the relation `relName` already has an entry in the Open Relation Table */
    // (checked using OpenRelTable::getRelId())

    // return that relation id;
  int relid = OpenRelTable::getRelId(relName); 

  /* find a free slot in the Open Relation Table
     using OpenRelTable::getFreeOpenRelTableEntry(). */

  if (/* free slot not available */){
    return E_CACHEFULL;
  }

  // let relId be used to store the free slot.
  int relId;

  /****** Setting up Relation Cache entry for the relation ******/

  /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/

  // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
  RecId relcatRecId;

  if (/* relcatRecId == {-1, -1} */) {
    // (the relation is not found in the Relation Catalog.)
    return E_RELNOTEXIST;
  }

  /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      update the recId field of this Relation Cache entry to relcatRecId.
      use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  */

  /****** Setting up Attribute Cache entry for the relation ******/

  // let listHead be used to hold the head of the linked list of attrCache entries.
  AttrCacheEntry* listHead;

  /*iterate over all the entries in the Attribute Catalog corresponding to each
  attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
  care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
  corresponding to Attribute Catalog before the first call to linearSearch().*/
  {
      /* let attrcatRecId store a valid record id an entry of the relation, relName,
      in the Attribute Catalog.*/
      RecId attrcatRecId;

      /* read the record entry corresponding to attrcatRecId and create an
      Attribute Cache entry on it using RecBuffer::getRecord() and
      AttrCacheTable::recordToAttrCatEntry().
      update the recId field of this Attribute Cache entry to attrcatRecId.
      add the Attribute Cache entry to the linked list of listHead .*/
      // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
  }

  // set the relIdth entry of the AttrCacheTable to listHead.

  /****** Setting up metadata in the Open Relation Table for the relation******/

  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.

  return relId;
}




