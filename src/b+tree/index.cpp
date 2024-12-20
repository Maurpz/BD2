#include <bits/stdc++.h>
#include <iostream>
#include <random>
#include <utility>
#include <vector>
using namespace std;

#ifndef INDEX_H 
#define INDEX_H


// Definición de la estructura CTID
struct CTID {
    int numBlock;
    int offsetTuple;
};

// Clase Node para el B+ Tree
class Node {
public:
    vector<int> keys;               // Claves en el nodo
    Node* parent;                   // Nodo padre
    vector<Node*> children;         // Hijos (solo para nodos internos)
    vector<CTID> values;            // Valores (solo para hojas)
    Node* next;                     // Siguiente hoja
    Node* prev;                     // Hoja anterior
    bool isLeaf;                    // Indica si el nodo es una hoja

    // Constructor
    Node(Node* parent = nullptr, bool isLeaf = false, Node* prev_ = nullptr, Node* next_ = nullptr)
        : parent(parent), isLeaf(isLeaf), prev(prev_), next(next_) {
        if (next_) next_->prev = this;
        if (prev_) prev_->next = this;
    }

    // Retorna el índice del hijo adecuado para una clave
    int indexOfChild(int key) const {
        int i = 0;
        while (i < (int)keys.size() && key >= keys[i]) i++;
        return i;
    }

    // Retorna el índice de una clave
    int indexOfKey(int key) const {
        for (int i = 0; i < (int)keys.size(); i++)
            if (keys[i] == key) return i;
        return -1;
    }

    // Obtiene el hijo correspondiente a una clave
    Node* getChild(int key) const {
        return children[indexOfChild(key)];
    }

    // Inserta una clave y actualiza los hijos
    void setChild(int key, const vector<Node*>& newChildren) {
        int i = indexOfChild(key);
        keys.insert(keys.begin() + i, key);
        // Remover el hijo que se está dividiendo
        children.erase(children.begin() + i);
        // Insertar los nuevos hijos
        children.insert(children.begin() + i, newChildren.begin(), newChildren.end());
        // Actualizar los padres de los nuevos hijos
        for(auto child : newChildren) {
            if(child) child->parent = this;
        }
    }

    // Divide un nodo hoja
    tuple<int, Node*, Node*> splitLeaf() {
        int mid = (int)keys.size() / 2;
        Node* left = new Node(parent, true, prev, this);
        left->keys.assign(keys.begin(), keys.begin() + mid);
        left->values.assign(values.begin(), values.begin() + mid);

        keys.erase(keys.begin(), keys.begin() + mid);
        values.erase(values.begin(), values.begin() + mid);

        if (prev) prev->next = left;
        prev = left;

        return {keys[0], left, this};
    }

    // Divide un nodo interno
    tuple<int, Node*, Node*> splitInternal() {
        int mid = (int)keys.size() / 2;
        int upKey = keys[mid];

        Node* left = new Node(parent, false);
        left->keys.assign(keys.begin(), keys.begin() + mid);
        left->children.assign(children.begin(), children.begin() + mid + 1);
        for(auto child : left->children) {
            if(child) child->parent = left;
        }

        Node* right = new Node(parent, false);
        right->keys.assign(keys.begin() + mid + 1, keys.end());
        right->children.assign(children.begin() + mid + 1, children.end());
        for(auto child : right->children) {
            if(child) child->parent = right;
        }

        keys.erase(keys.begin(), keys.begin() + mid + 1);
        children.erase(children.begin(), children.begin() + mid + 1);

        return {upKey, left, right};
    }

    // Obtiene el CTID asociado a una clave
    CTID get(int key) const {
        int idx = indexOfKey(key);
        if (idx != -1) return values[idx];
        throw invalid_argument("Clave no encontrada");
    }

    // Inserta o actualiza una clave y su valor
    void set(int key, const CTID& value) {
        int i = indexOfChild(key);
        if (i < (int)keys.size() && keys[i] == key) {
            values[i] = value; // Actualizar valor existente
        } else {
            keys.insert(keys.begin() + i, key);
            values.insert(values.begin() + i, value);
        }
    }

};


















// Clase BPlusTree
class BPlusTree {
public:
    Node* root;
    int maxCapacity;
    int minCapacity;
    int depth;

    // Constructor
    BPlusTree(int _maxCapacity = 4) {
        root = new Node(nullptr, true, nullptr, nullptr);
        maxCapacity = _maxCapacity > 2 ? _maxCapacity : 2;
        minCapacity = (int)ceil(maxCapacity / 2.0);
        depth = 0;
    }

    // Encuentra la hoja que debe contener la clave
    Node* findLeaf(int key) const {
        Node* node = root;
        while (!node->isLeaf) {
            node = node->getChild(key);
        }
        return node;
    }

    // Obtiene el CTID asociado a una clave
    CTID get(int key) const {
        return findLeaf(key)->get(key);
    }

    // Inserta o actualiza una clave con su CTID
    void set(int key, const CTID& value) {
        Node* leaf = findLeaf(key);
        leaf->set(key, value);
        if ((int)leaf->keys.size() > maxCapacity) {
            insert(leaf->splitLeaf());
        }
    }

    // Actualiza el valor de una clave existente
    void update(int key, const CTID& newValue) {
        Node* leaf = findLeaf(key);
        int idx = leaf->indexOfKey(key);
        if (idx == -1) {
            throw invalid_argument("Clave no encontrada para actualizar");
        }
        leaf->values[idx] = newValue;
    }

    // Maneja la inserción después de una división
    void insert(const tuple<int, Node*, Node*>& result) {
        int key = std::get<0>(result);
        Node* left = std::get<1>(result);
        Node* right = std::get<2>(result);
        Node* parent = right->parent;

        if (parent == nullptr) {
            // Crear una nueva raíz
            root = new Node(nullptr, false, nullptr, nullptr);
            root->keys.push_back(key);
            root->children.push_back(left);
            root->children.push_back(right);
            left->parent = root;
            right->parent = root;
            depth += 1;
            return;
        }

        // Insertar la clave y actualizar los hijos en el padre
        parent->setChild(key, {left, right});
        if ((int)parent->keys.size() > maxCapacity) {
            insert(parent->splitInternal());
        }
    }

    // --- Funciones auxiliares para remove ---

    void removeFromLeaf(int key, Node* node) {
        int index = node->indexOfKey(key);
        if (index == -1) {
            throw invalid_argument("Key no encontrada en removeFromLeaf");
        }
        node->keys.erase(node->keys.begin() + index);
        node->values.erase(node->values.begin() + index);
    }

    void removeFromInternal(int key, Node* node) {
        int index = node->indexOfKey(key);
        if (index != -1) {
            // Reemplazar la clave interna por la siguiente mayor (en la hoja)
            Node* rightChild = node->children[index + 1];
            while (!rightChild->isLeaf) {
                rightChild = rightChild->children.front();
            }
            int newKey = rightChild->keys.front();
            node->keys[index] = newKey;
        }
    }

    void borrowKeyFromRightLeaf(Node* node, Node* next) {
        node->keys.push_back(next->keys.front());
        node->values.push_back(next->values.front());
        next->keys.erase(next->keys.begin());
        next->values.erase(next->values.begin());

        // Actualizar la clave en el padre
        int pos = -1;
        for (int i = 0; i < (int)node->parent->children.size(); i++) {
            if (node->parent->children[i] == next) {
                pos = i;
                break;
            }
        }
        if (pos > 0 && pos - 1 < (int)node->parent->keys.size()) {
            node->parent->keys[pos - 1] = next->keys.front();
        }
    }

    void borrowKeyFromLeftLeaf(Node* node, Node* prev) {
        node->keys.insert(node->keys.begin(), prev->keys.back());
        node->values.insert(node->values.begin(), prev->values.back());
        prev->keys.pop_back();
        prev->values.pop_back();

        // Actualizar la clave en el padre
        int pos = -1;
        for (int i = 0; i < (int)prev->parent->children.size(); i++) {
            if (prev->parent->children[i] == prev) {
                pos = i;
                break;
            }
        }
        if (pos != -1 && pos < (int)prev->parent->keys.size()) {
            node->parent->keys[pos] = node->keys.front();
        }
    }

    void mergeNodeWithRightLeaf(Node* node, Node* next) {
        node->keys.insert(node->keys.end(), next->keys.begin(), next->keys.end());
        node->values.insert(node->values.end(), next->values.begin(), next->values.end());
        node->next = next->next;
        if (node->next) node->next->prev = node;

        int pos = -1;
        for (int i = 0; i < (int)node->parent->children.size(); i++) {
            if (node->parent->children[i] == next) {
                pos = i;
                break;
            }
        }

        if (pos > 0 && pos - 1 < (int)node->parent->keys.size()) {
            node->parent->keys.erase(node->parent->keys.begin() + pos - 1);
        }
        node->parent->children.erase(node->parent->children.begin() + pos);
    }

    void mergeNodeWithLeftLeaf(Node* node, Node* prev) {
        prev->keys.insert(prev->keys.end(), node->keys.begin(), node->keys.end());
        prev->values.insert(prev->values.end(), node->values.begin(), node->values.end());
        prev->next = node->next;
        if (prev->next) prev->next->prev = prev;

        int pos = -1;
        for (int i = 0; i < (int)prev->parent->children.size(); i++) {
            if (prev->parent->children[i] == node) {
                pos = i;
                break;
            }
        }
        if (pos > 0 && pos - 1 < (int)prev->parent->keys.size()) {
            prev->parent->keys.erase(prev->parent->keys.begin() + pos - 1);
        }
        prev->parent->children.erase(prev->parent->children.begin() + pos);
    }

    void borrowKeyFromRightInternal(int myPositionInParent, Node* node, Node* next) {
        node->keys.push_back(root->keys[myPositionInParent]);
        root->keys[myPositionInParent] = next->keys.front();
        next->keys.erase(next->keys.begin());

        node->children.push_back(next->children.front());
        next->children.erase(next->children.begin());
        node->children.back()->parent = node;
    }

    void borrowKeyFromLeftInternal(int myPositionInParent, Node* node, Node* prev) {
        node->keys.insert(node->keys.begin(), root->keys[myPositionInParent - 1]);
        root->keys[myPositionInParent - 1] = prev->keys.back();
        prev->keys.pop_back();

        node->children.insert(node->children.begin(), prev->children.back());
        prev->children.pop_back();
        node->children.front()->parent = node;
    }

    void mergeNodeWithRightInternal(int myPositionInParent, Node* node, Node* next) {
        node->keys.push_back(root->keys[myPositionInParent]);
        root->keys.erase(root->keys.begin() + myPositionInParent);
        root->children.erase(root->children.begin() + myPositionInParent + 1);

        node->keys.insert(node->keys.end(), next->keys.begin(), next->keys.end());
        node->children.insert(node->children.end(), next->children.begin(), next->children.end());
        for (auto child : node->children) {
            if (child) child->parent = node;
        }
    }

    void mergeNodeWithLeftInternal(int myPositionInParent, Node* node, Node* prev) {
        prev->keys.push_back(root->keys[myPositionInParent - 1]);
        root->keys.erase(root->keys.begin() + myPositionInParent - 1);
        root->children.erase(root->children.begin() + myPositionInParent);

        prev->keys.insert(prev->keys.end(), node->keys.begin(), node->keys.end());
        prev->children.insert(prev->children.end(), node->children.begin(), node->children.end());
        for (auto child : prev->children) {
            if (child) child->parent = prev;
        }
    }

    // Función remove principal
    void remove(int key, Node* node = nullptr) {
        if (node == nullptr) {
            node = findLeaf(key);
        }
        
        // Eliminar clave en hoja o en interno
        if (node->isLeaf) {
            removeFromLeaf(key, node);
        } else {
            removeFromInternal(key, node);
        }

        // Verificar si el nodo cumple con minCapacity
        if ((int)node->keys.size() < minCapacity && node != root) {
            if (node->isLeaf) {
                Node* next = node->next;
                Node* prev = node->prev;

                if (next && next->parent == node->parent && (int)next->keys.size() > minCapacity) {
                    borrowKeyFromRightLeaf(node, next);
                } else if (prev && prev->parent == node->parent && (int)prev->keys.size() > minCapacity) {
                    borrowKeyFromLeftLeaf(node, prev);
                } else if (next && next->parent == node->parent && (int)next->keys.size() <= minCapacity) {
                    mergeNodeWithRightLeaf(node, next);
                } else if (prev && prev->parent == node->parent && (int)prev->keys.size() <= minCapacity) {
                    mergeNodeWithLeftLeaf(node, prev);
                }
            } else {
                // Nodo interno
                Node* parent = node->parent;
                int myPositionInParent = -1;
                for (int i = 0; i < (int)parent->children.size(); i++) {
                    if (parent->children[i] == node) {
                        myPositionInParent = i;
                        break;
                    }
                }

                Node* next = (myPositionInParent + 1 < (int)parent->children.size()) 
                             ? parent->children[myPositionInParent + 1] : nullptr;
                Node* prev = (myPositionInParent - 1 >= 0) 
                             ? parent->children[myPositionInParent - 1] : nullptr;

                if (next && next->parent == parent && (int)next->keys.size() > minCapacity) {
                    borrowKeyFromRightInternal(myPositionInParent, node, next);
                } else if (prev && prev->parent == parent && (int)prev->keys.size() > minCapacity) {
                    borrowKeyFromLeftInternal(myPositionInParent, node, prev);
                } else if (next && next->parent == parent && (int)next->keys.size() <= minCapacity) {
                    mergeNodeWithRightInternal(myPositionInParent, node, next);
                } else if (prev && prev->parent == parent && (int)prev->keys.size() <= minCapacity) {
                    mergeNodeWithLeftInternal(myPositionInParent, node, prev);
                }
            }
        }

        // Ajustar la raíz si es necesario
        if (root->keys.empty() && !root->isLeaf) {
            if (!root->children.empty()) {
                root = root->children[0];
                root->parent = nullptr;
                depth = max(0, depth - 1);
            }
        }
    }

    // Función para imprimir el árbol
    void print(Node* node = nullptr, string _prefix = "", bool _last = true) const {
        if (!node) node = root;
        cout << _prefix;
        cout << (_last ? "└─ " : "├─ ");
        cout << "[";
        for (int i = 0; i < (int)node->keys.size(); i++) {
            cout << node->keys[i];
            if (i != (int)node->keys.size() - 1) cout << ", ";
        }
        cout << "]\n";

        _prefix += _last ? "    " : "│   ";

        if (!node->isLeaf) {
            for (int i = 0; i < (int)node->children.size(); i++) {
                bool isLast = (i == (int)node->children.size() - 1);
                print(node->children[i], _prefix, isLast);
            }
        }
    }
};

#endif
