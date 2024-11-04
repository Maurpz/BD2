#include "Buffer.h"

Buffer::Buffer(int fileNodeOID, int pageNum) {
  this->data = new char[PAGE_SIZE];
  this->dirty = false;
  this->pinCount = 0;
  this->fileNodeOID = fileNodeOID;
  this->pageNum = pageNum;
}

Buffer::~Buffer(){
  delete [] data;
}

char * Buffer::getData() {
  return data;
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

void Buffer::setData(char data[]) {

}