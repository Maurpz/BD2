#include "Catalog.cpp"
using namespace std;

int main() {

  Catalog c1;
  c1.createTable("profes");
  unique_ptr<PgClassRow> res = c1.getTable("profes");
  c1.createColumn(res->oid,"id","INT",1,false);
  return 0;
}