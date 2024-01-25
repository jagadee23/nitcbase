#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
#include <cstring>
using namespace std;


int main(int argc, char *argv[])
{
    Disk disk_run;
    StaticBuffer buffer;
    //cout << "main1\n";
    OpenRelTable cache;
    //cout << "main\n";
    //return FrontendInterface::handleFrontend(argc, argv);
    for(int i=0; i<2; i++){
        RelCatEntry relcatentry;
        RelCacheTable::getRelCatEntry(i, &relcatentry);
        cout << "Relation: " << relcatentry.relName << endl;
        for(int j=0; j<relcatentry.numAttrs; j++){
            AttrCatEntry attrcatentry;
            AttrCacheTable::getAttrCatEntry(i, j, &attrcatentry);
            cout << attrcatentry.attrName << ' ' << (attrcatentry.attrType==NUMBER?"NUM":"STR") << endl;
        }
        cout<<endl;
    }
    return 0;
}

/*STAGE 1 :
 unsigned char buffer[BLOCK_SIZE];
 Disk::readBlock(buffer,7000);
 char msg[] ="hello";
 memcpy(buffer+800,msg,6);
 Disk::writeBlock(buffer,7000);
 unsigned char buffer2[BLOCK_SIZE];
 Disk::readBlock(buffer2,7000);
 char msg2[6];
 memcpy(msg2,buffer2+800,6);
 cout<<msg2<<endl;*/
/*unsigned char buf[BLOCK_SIZE];
Disk::readBlock(buf,0);
for(auto i: buf){
  cout<<int(i)<<" ";
}*/

//   RecBuffer relCatBuffer(RELCAT_BLOCK);
//   RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

//   HeadInfo relCatHeader;
//   HeadInfo attrCatHeader;

//   // load the headers of both the blocks into relCatHeader and attrCatHeader.
//   // (we will implement these functions later)
//   relCatBuffer.getHeader(&relCatHeader);
//   attrCatBuffer.getHeader(&attrCatHeader);

//   Attribute relCatRecord[RELCAT_NO_ATTRS];
//   Attribute attrCatRecord[ATTRCAT_NO_ATTRS]; // declare attrCatRecord outside the loop

//   for (int i = 0; i < relCatHeader.numEntries; i++) {
//     relCatBuffer.getRecord(relCatRecord, i);

//     printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

//     for (int j = 0; j < attrCatHeader.numEntries; j++) {
//     attrCatBuffer.getRecord(attrCatRecord, j);

//     // Check if attribute catalog entry corresponds to the current relation
//     if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0) {
//         const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
//         printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
//     }
// }
/*const char* relName="Students"; const char* oldAttrName="Class";const char* newAttrName="Batch";
RecBuffer attrCatBuffer (ATTRCAT_BLOCK);
	
	HeadInfo attrCatHeader;
	attrCatBuffer.getHeader(&attrCatHeader);

	// iterating the records in the Attribute Catalog
	// to find the correct entry of relation and attribute
	for (int recIndex = 0; recIndex < attrCatHeader.numEntries; recIndex++) {
		Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
		attrCatBuffer.getRecord(attrCatRecord, recIndex);

		// matching the relation name, and attribute name
		if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relName) == 0
			&& strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, oldAttrName) == 0) 
		{
			strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newAttrName);
			attrCatBuffer.setRecord(attrCatRecord, recIndex);
			std::cout << "Update successful!\n\n";
			break;
		}

		// reaching at the end of the block, and thus loading
		// the next block and setting the attrCatHeader & recIndex
		if (recIndex == attrCatHeader.numSlots-1) {
			recIndex = -1;
			attrCatBuffer = RecBuffer (attrCatHeader.rblock);
			attrCatBuffer.getHeader(&attrCatHeader);
		}
	}*/
//     printf("\n");
//   }
