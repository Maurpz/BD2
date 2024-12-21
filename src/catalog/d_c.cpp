#include "Catalog.cpp"
// #include "../executor/Executor.cpp"

#include "../buffer/BufferManager.cpp"
//#include "../disck/DisckManager.cpp"

#include "../executor/executor2.cpp"

int main() {
  DisckManager dm(2,4,4,4,512,2);
  // dm.createDirectories();
  Catalog c1(&dm);
  BufferManager b1(&dm);

  Executor e1;
  e1.setReferenceToCatalog(&c1);
  e1.setReferenceToBufferManager(&b1);

  string datalorem = "Contrary to popular ";


  string nameTable = "peques";
  // c1.createTable(nameTable);
  unique_ptr<PgClassRow> res = c1.getTable(nameTable);
  // c1.createColumn(res->oid, "id", "INT", 1, false);
  // c1.createColumn(res->oid, "nombre", "VARCHAR", 2, false);
  // b1.newPage(res->oid, 0);


  vector<Col_Data> dataToInsert;
  Col_Data d1;
  d1.columnName = "id";
  d1.data = "14";

  dataToInsert.push_back(d1);

  Col_Data d2;
  d2.columnName = "nombre";
  d2.data = "sdfff";

  dataToInsert.push_back(d2);

  e1.insertInto(nameTable, dataToInsert);
  //b1.flushPage(res->oid, 0);

  e1.selectAll(nameTable);



  // cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@q"<<endl;
  // cout<<"numero de columnas de la tabla"<<c1.getNumColumns(res->oid)<<endl;
  // cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@q"<<endl;

  // Executor e1;


  return 0;
};