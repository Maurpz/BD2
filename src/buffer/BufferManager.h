
using namespace std;

#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H
#include "Buffer.h"
#include <map>
#include <fstream>
#include <memory>


class BufferManager {
  private:
    map<pair<int,int>,unique_ptr<Buffer>> bufferPool;
    unique_ptr <fstream> r_and_w;
    int maxSize;
    
  public:
    BufferManager();
    Buffer * getPage(int fileNodeOID, int pageNum);
    Buffer * newPage(int fileNodeOID);
    bool addPage(Buffer newPage, int oid);
    bool existsPage(pair<int,int> key);

};
#endif