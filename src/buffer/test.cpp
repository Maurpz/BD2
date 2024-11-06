#include "Buffer.cpp"
#include "BufferManager.cpp"

int main () {
  BufferManager b1;
  b1.setDbName("testDB");
  b1.setHostName("db_storage");
  b1.newPage(12133);
  b1.printStatus();
  b1.newPage(12133);
  b1.printStatus();


  return 0;
}