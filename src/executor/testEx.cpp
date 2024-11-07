#include "Executor.cpp"
#include "../buffer/BufferManager.cpp"
#include "../catalog/Catalog.cpp"


int main () {
  BufferManager b1;
  b1.setDbName("testDB");
  b1.setHostName("db_storage");
  //b1.newPage(514683);
  Catalog c1;

  Executor exe;
  exe.setReferenceToBufferManager(&b1);
  exe.setReferenceToCatalog(&c1);

  exe.executeInsert1("alumnos","edad",12);
  return 0;
}