#include "Buffer.h"
#include <cstring>

Buffer::Buffer(int fileNodeOID, int pageNum) {
  this->data = make_unique<char []>(PAGE_SIZE);
  // this->data = new char[PAGE_SIZE];
  // memset(data,0,PAGE_SIZE);
  memset(data.get(),0,PAGE_SIZE);
  this->dirty = false;
  this->pinCount = 0;
  this->fileNodeOID = fileNodeOID;
  this->pageNum = pageNum;
}

Buffer::~Buffer(){
  //delete [] data;
}

/*posible mejora propuesta
Buffer::Buffer(int fileNodeOID, int pageNum) : data(nullptr), dirty(false), pinCount(0), fileNodeOID(fileNodeOID), pageNum(pageNum) {
    data = new char[PAGE_SIZE];
}

Buffer::~Buffer() {
    delete[] data;
}
*/

char * Buffer::getData() {
  return data.get();
}

int Buffer::getFileNodeOID() {
  return this->fileNodeOID;
}

bool Buffer::isDirty() {
  return this->dirty;
}

void Buffer::setIsDirty(bool changeDirty) {
  this->dirty = changeDirty;
}

void Buffer::setData(unique_ptr<char []> data) {
  this->data = move(data);

}