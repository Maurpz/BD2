#include "Storage.h"
#include <random>
using namespace std;

int Storage::generateOID() {
  random_device rd;
  mt19937 gen(rd());

  uniform_int_distribution<> distrib(500000,700000);
  int numero_aleatorio = distrib(gen);
  return numero_aleatorio;
}