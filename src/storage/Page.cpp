#include "Page.h"
#include <iostream>

int PageManager::generatebitMapNull (int size) {
  int bitmap = 0;
  for ( int i = 0; i < size; i++) {
    bitmap |= 1 << i;
  }
  return bitmap;
}

void PageManager::changeBitMap(int * bitmap, u_int16_t & index) {
  *bitmap &= ~(1 << (index-1));
}

bool PageManager::isBitNull(uint16_t & bitmap, uint16_t  & index) {
  return (bitmap >> (index-1)) & 1;
}

