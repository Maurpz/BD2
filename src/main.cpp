#include "./catalog/Catalog.cpp"
#include "./buffer/BufferManager.cpp"
#include "./executor/executor2.cpp"

int main() {
  DisckManager dm(2,4,4,4,512,2);
  dm.createDirectories();

  Catalog c1(&dm);
  BufferManager b1(&dm,10);
  Executor e1;


  e1.setReferenceToCatalog(&c1);
  e1.setReferenceToBufferManager(&b1);

  string nameTable = "mary44";

  c1.createTable(nameTable);
  unique_ptr<PgClassRow> table = c1.getTable(nameTable);

  c1.createColumn(table->oid, "id", "INT", 1, true);
  c1.createColumn(table->oid, "nombre", "VARCHAR", 2, true);
  b1.newPage(table->oid, 0);

  c1.createIndex(nameTable, "id");

  //!data to insert

  vector<Col_Data> data;
  Col_Data d1, d2;
  d1.columnName = "id";
  d1.data = "1";

  d2.columnName = "nombre";
  d2.data = "Jose";

  data.push_back(d1);
  data.push_back(d2);


  e1.insertInto(nameTable, data);




  //! select where
  Condition cond1;
  cond1.column = "id";
  cond1.operator_ = "=";
  cond1.value = "1";

  vector<Condition> conds;
  conds.push_back(cond1);
  e1.selectAllWhere(nameTable, conds);


  return 0;
}
