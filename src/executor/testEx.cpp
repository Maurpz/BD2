#include "Executor.cpp"
#include "../buffer/BufferManager.cpp"
#include "../catalog/Catalog.cpp"



//*main para crear y leer datosoa una tabla de 1 solo registro de longitud fija
/*
int main () {
  BufferManager b1;
  b1.setDbName("testDB");
  b1.setHostName("db_storage");
  Catalog c1;

  // c1.createTable("alumnos");
  // unique_ptr<PgClassRow> res = c1.getTable("alumnos");
  // c1.createColumn(res->oid,"edad","INT",1,false);
  // b1.newPage(res->oid);
  // b1.printStatus();

  Executor exe;
  exe.setReferenceToBufferManager(&b1);
  exe.setReferenceToCatalog(&c1);

  // exe.executeInsert1("alumnos","edad",14);
  // exe.executeInsert1("alumnos","edad",455);
  // exe.executeInsert1("alumnos","edad",987645);
  // b1.flushPage(res->oid,0);
  exe.executeInsert1("alumnos","edad",141414);
  exe.executeSelectAll1("alumnos");
  return 0;
}
*/

/*

//* tabla con 2 campos fijos
int main () {
  BufferManager b1;
  b1.setDbName("testDB");
  b1.setHostName("db_storage");
  Catalog c1;
  Executor exe;
  exe.setReferenceToBufferManager(&b1);
  exe.setReferenceToCatalog(&c1);

  // c1.createTable("alumnos");
  // unique_ptr<PgClassRow> res = c1.getTable("alumnos");
  // c1.createColumn(res->oid,"edad","INT",1,false);
  // b1.newPage(res->oid);
  // b1.printStatus();

  string tableName = "profesores";
  //c1.createTable(tableName);
  //unique_ptr<PgClassRow> res = c1.getTable(tableName);

  //c1.createColumn(res->oid, "id","INT",1,false);
  //c1.createColumn(res->oid, "dni","INT",1,false);


  //b1.newPage(res->oid);


  exe.executeInsert2(tableName, "id",1,"dni",23456);
  exe.executeInsert2(tableName, "id",2,"dni",111111);
  exe.executeInsert2(tableName, "id",3,"dni",444444);

  exe.executeSelectAll2(tableName);
  return 0;
}
*/

/*
int main () {
  BufferManager b1;
  b1.setDbName("testDB");
  b1.setHostName("db_storage");
  Catalog c1;
  Executor exe;
  exe.setReferenceToBufferManager(&b1);
  exe.setReferenceToCatalog(&c1);

  string nameTable = "universidades";

  // c1.createTable(nameTable);;
  // unique_ptr<PgClassRow> res = c1.getTable(nameTable);
  // c1.createColumn(res->oid, "nombre", "VARCHAR", 1, false);
  // b1.newPage(res->relfilenode);

  exe.executeInsertString(nameTable, "nombre", "Juan de halla de la torre");
  exe.executeInsertString(nameTable, "nombre", "pepito");
  exe.executeInsertString(nameTable, "nombre", "maria");

  exe.executeSelectAllStrings(nameTable);
  return 0;
}
*/

//*creamos tabla con campos de diferentes tipos

int main () {
  BufferManager b1;
  b1.setDbName("testDB");
  b1.setHostName("db_storage");
  Catalog c1;
  Executor exe;
  exe.setReferenceToBufferManager(&b1);
  exe.setReferenceToCatalog(&c1);

  string nameTable = "users";

  c1.createTable(nameTable);
  unique_ptr<PgClassRow> res = c1.getTable(nameTable);

  c1.createColumn(res->oid, "id", "INT", 1, false);
  c1.createColumn(res->oid, "nombre", "VARCHAR", 2, false);
  c1.createColumn(res->oid, "apellido", "VARCHAR", 3, false);
  c1.createColumn(res->oid, "edad", "INT", 4, false);

  b1.newPage(res->relfilenode);

  vector<Col_Data> data;
  Col_Data d1, d2, d3, d4;
  d1.columnName = "id";
  d1.data = "1";

  d2.columnName = "nombre";
  d2.data = "Jose";

  d3.columnName = "apellido";
  d3.data = "Hualpa";

  d4.columnName = "edad";
  d4.data = "24";
  data.push_back(d3);
  data.push_back(d2);
  data.push_back(d1);
  data.push_back(d4);

  vector<Col_Data> data1;
  Col_Data d11, d12, d13, d14;
  d11.columnName = "id";
  d11.data = "2";

  d12.columnName = "nombre";
  d12.data = "Valeria";

  d13.columnName = "apellido";
  d13.data = "Chavez";

  d14.columnName = "edad";
  d14.data = "24";
  data1.push_back(d14);
  data1.push_back(d12);
  data1.push_back(d13);
  data1.push_back(d11);


  vector<Col_Data> data2;
  Col_Data d21, d22, d24, d23;
  d21.columnName = "id";
  d21.data = "3";

  d22.columnName = "nombre";
  d22.data = "Norka";

  d24.columnName = "edad";
  d24.data = "23";

  // d23.columnName = "apellido";
  // d23.data = "Lopez";

  data2.push_back(d24);
  data2.push_back(d22);
  data2.push_back(d21);



  //*INSERT
  cout<<"INSERT"<<endl;
  exe.insertInto(nameTable, data);
  exe.insertInto(nameTable, data1);
  exe.insertInto(nameTable, data2);

  // exe.selectAll(nameTable);
  vector<string> columns;
  columns.push_back("nombre");
  columns.push_back("apellido");
  //*select all
  cout<<"SELECT ALL"<<endl;
  exe.selectAll(nameTable);

  //*SELECT CUSTOM
  cout<<"SELECT CUSTOM"<<endl;
  exe.selectCustom(nameTable, columns);


  vector<Condition> conds;
  Condition cond1;
  cond1.column = "apellido";
  cond1.operator_ = "=";
  cond1.value = "Hualpa";
  conds.push_back(cond1);
  //exe.selectAllWhere(nameTable, conds);

  //exe.selectAllWherev2(nameTable,conds);
  //exe.selectAll(nameTable);

  vector<pair<string,string>> columnAndValue;
  columnAndValue.push_back(make_pair("nombre","Pepito Quintanilla"));
  columnAndValue.push_back(make_pair("apellido","Noah"));
  columnAndValue.push_back(make_pair("edad","19"));






  vector<pair<string,string>> columnAndValue2;
  columnAndValue2.push_back(make_pair("edad","45"));

  vector<Condition> conds2;
  Condition cond2;
  cond2.column = "edad";
  cond2.operator_ = ">";
  cond2.value = "2";
  conds2.push_back(cond2);


  vector<Condition> conds3;
  Condition cond23;
  cond23.column = "edad";
  cond23.operator_ = "=";
  cond23.value = "45";
  conds3.push_back(cond23);

  //* update a
  cout<<"update tabla set (nombre,apellido,edad) where  apellido=hualpa "<<endl;

  exe.updateRegister(nameTable, columnAndValue, conds,true);
  cout<<"SELECT ALL"<<endl;
  exe.selectAll(nameTable);
  cout<<"UPDATE tabla edad=45 where  edad > 2"<<endl;
  exe.updateRegister(nameTable, columnAndValue2, conds2,false);
  cout<<"SELECT ALL"<<endl;
  exe.selectAll(nameTable);
  cout<<"DELETE from tabla where edad = 45"<<endl;
  exe.deleteRegister(nameTable, conds3, false);
  cout<<"SELECT ALL"<<endl;
  exe.selectAll(nameTable);


  return 0;
}

