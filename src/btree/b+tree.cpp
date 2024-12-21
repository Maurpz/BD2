#include <iostream>
#include <vector>
using namespace std;

const int Orden = 4;
struct BPlusTree {
  bool isLeaf;
  int key[Orden -1];
  BPlusTree * hijos [Orden];
  int values [Orden-1];
  BPlusTree * next;

  BPlusTree(bool leaf = true) {
    isLeaf = leaf;
    next = nullptr;
  }
};

// BPlusTree * insert(struct BPlusTree * root, int key, int value) {
//   if (root->isLeaf) {
//     root->keys.push_back(key);
//     root->values.push_back(value);
//     return root;
//   }
//   if ()

// }




int main () {
  BPlusTree * root = nullptr;

  return 0;
}