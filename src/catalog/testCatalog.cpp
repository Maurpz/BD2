#include "Catalog.cpp"
using namespace std;

int main() {

  Catalog c1;
  c1.createTable("alumnos");
  unique_ptr<PgClassRow> res = c1.getTable("alumnos");
  c1.createColumn(res->oid,"edad","INT",1,false);
  return 0;
}