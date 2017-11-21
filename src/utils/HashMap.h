#ifndef HASHMAP_H
#define HASHMAP_H

#include <Arduino.h>

#define BUFFER_SIZE 10

#ifndef NULL
#define NULL 0
#endif

template<class T, class R>
struct item {
  T key;
  R value;
  item *next;
};

template<class T, class R>
class HashMapIterator;

template<class T, class R>
class HashMap {
  friend class HashMapIterator<T, R>;

  private:
    int len;
    item<T, R> *buffer[BUFFER_SIZE];

    int calculateHash(T key) {
      String strKey = String(key);
      int hash = 0;
      for (int i = 0; i < strKey.length(); i++) {
        hash += strKey.charAt(i);
      }
      return hash % BUFFER_SIZE;
    }

  public:
    HashMap() {
      len = 0;
      for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = NULL;
      }
    }

    ~HashMap() {
      clear();
    }

    void put(T key, R value) {
      int hash = calculateHash(key);
      bool match = 0;
      if (buffer[hash] == NULL) {
        item<T,R> *tmp = new item<T,R>;
        tmp->key = key;
        tmp->value = value;
        tmp->next = NULL;
        buffer[hash] = tmp;
        len++;
      } else {
        item<T,R> *cur = buffer[hash];
        item<T,R> *ant = NULL;
        do {
          if (cur->key == key) {
            cur->value = value;
            match = 1;
            break;
          }
          ant = cur;
          cur = cur->next;
        } while(cur != NULL);
        if (!match) {
          item<T,R> *tmp = new item<T,R>;
          tmp->key = key;
          tmp->value = value;
          tmp->next = NULL;
          ant->next = tmp;
          len++;
        }
      }
    }

    R get(T key) {
      int hash = calculateHash(key);
      item<T,R> *cur = buffer[hash];
      while (cur != NULL && cur->key != key) {
          cur = cur->next;
      }
      if (cur != NULL) {
        return cur->value;
      }
    }

    bool remove(T key) {
      int hash = calculateHash(key);
      item<T,R> *cur = buffer[hash];
      item<T,R> *ant = NULL;
      while (cur != NULL && cur->key != key) {
        ant = cur;
        cur = cur->next;
      }
      if (cur != NULL) {
        if (ant == NULL) {
          buffer[hash] = cur->next;
          cur->next = NULL;
          delete cur;
        } else {
          ant->next = cur->next;
          cur->next = NULL;
          delete cur;
        }
        len--;
        return 1;
      }
      return 0;
    }

    int size() {
      return len;
    }

    void clear() {
      for(int i = 0; i < BUFFER_SIZE; i++) {
        if (buffer[i] != NULL) {
          item<T,R> *cur = buffer[i];
          while(cur != NULL) {
            item<T,R> *tmp = cur->next;
            delete cur;
            cur = tmp;
          }
        }
      }
      len = 0;
    }
};

template<class T, class R>
class HashMapIterator {
private:
  HashMap<T, R> hashMap;
  int idxBuffer;
  item<T,R> *cur;

public:
  HashMapIterator(HashMap<T, R> &hm) {
    hashMap = hm;
    idxBuffer = 0;
    cur = NULL;
  }

  bool next() {
    return 1;
  }

  T currentKey() {

  }


  R currentValue() {

  }
};

#endif
