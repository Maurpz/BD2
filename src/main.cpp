#include "./disck/DisckManager.cpp"
#include "./catalog/Catalog.cpp"
#include "./buffer/BufferManager.cpp"
#include "./executor/Executor.cpp"
#include "./b+tree/index.cpp"

int main() {
  DisckManager dm(2,4,4,4,512,2);

  Catalog c1(&dm);
  BufferManager b1(&dm);
  Executor e1;


  e1.setReferenceToCatalog(&c1);
  e1.setReferenceToBufferManager(&b1);


  return 0;
}
