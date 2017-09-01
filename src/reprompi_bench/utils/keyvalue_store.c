/*  ReproMPI Benchmark
 *
 *  Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
 Research Group for Parallel Computing
 Faculty of Informatics
 Vienna University of Technology, Austria

 <license>
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 </license>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keyvalue_store.h"

int reprompib_init_dictionary(reprompib_dictionary_t* hashtable, const int size) {
  int i;
  int ret = 1;

  if (size < 1) {
    return ret;
  }

  /* Allocate pointers to the head nodes. */
  if ((hashtable->table = calloc(size, sizeof(entry_t *))) == NULL) {
    return ret;
  }
  for (i = 0; i < size; i++) {
    hashtable->table[i] = NULL;
  }
  hashtable->size = size;

  return 0;
}

void reprompib_cleanup_dictionary(reprompib_dictionary_t* hashtable) {
  entry_t *last = NULL;
  entry_t *next = NULL;

  if (hashtable->table != NULL) {
    int i;
    for (i = 0; i < hashtable->size; i++) {
      next = hashtable->table[i];
      while (next != NULL) {
        free(next->key);
        free(next->value);
        last = next;
        next = next->next;
        free(last);
      }
    }
    free(hashtable->table);
  }
}

// One-at-a-Time Hash  by Bob Jenkins (http://www.burtleburtle.net/bob/hash/doobs.html)
static int ht_hash(const reprompib_dictionary_t *hashtable, const char* key) {
  unsigned long hash = 0;
  int i = 0;

  for (i = 0; i < strlen(key); i++) {
    hash += key[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);

  return (hash % hashtable->size);
}

static entry_t *ht_newpair(const char* key, const char* value) {
  entry_t *newpair;

  if (key == NULL || value == NULL) {
    return NULL;
  }

  if ((newpair = malloc(sizeof(entry_t))) == NULL) {
    return NULL;
  }

  newpair->key = strdup(key);
  newpair->value = strdup(value);
  newpair->next = NULL;

  return newpair;
}

int reprompib_add_element_to_dict(reprompib_dictionary_t* hashtable, const char* key, const char* value) {
  int bin = 0;
  entry_t *newpair = NULL;
  entry_t *next = NULL;
  entry_t *last = NULL;
  int ret = 0;

  if (key == NULL || value == NULL) {
    return 1;
  }

  bin = ht_hash(hashtable, key);
  next = hashtable->table[bin];

  while (next != NULL && strcmp(next->key, key) != 0) {
    last = next;
    next = next->next;
  }

  /* There's already a pair.  Let's replace that string. */
  if (next != NULL && strcmp(next->key, key) == 0) {
    next->value = strdup(value);

    /* Nope, could't find it.  Time to grow a pair. */
  } else {
    newpair = ht_newpair(key, value);

    if (newpair == NULL) {
      ret = 1;

      /* We're at the start of the linked list in this bin. */
    } else if (next == hashtable->table[bin]) {
      newpair->next = next;
      hashtable->table[bin] = newpair;

      /* We're at the end of the linked list in this bin. */
    } else if (next == NULL) {
      last->next = newpair;

      /* We're in the middle of the list. */
    } else {
      newpair->next = next;
      last->next = newpair;
    }
  }

  return ret;
}

int reprompib_get_value_from_dict(const reprompib_dictionary_t* hashtable, const char* key, char** value) {
  int bin = 0;
  entry_t *pair;
  int ret = 1;

  if (key == NULL) {
    *value = NULL;
    return ret;
  }

  bin = ht_hash(hashtable, key);

  /* Step through the bin, looking for our value. */
  pair = hashtable->table[bin];
  while (pair != NULL && strcmp(key, pair->key) != 0) {
    pair = pair->next;
  }

  /* Did we actually find anything? */
  if (pair == NULL || strcmp(key, pair->key) != 0) {
    *value = NULL;
  } else {
    *value = strdup(pair->value);
    ret = 0;
  }
  return ret;
}

int reprompib_remove_element_from_dict(reprompib_dictionary_t* hashtable, const char* key) {
  int bin = 0;
  int ret = 1;
  entry_t *pair;
  entry_t *last;

  if (key == NULL) {
    return ret;
  }

  bin = ht_hash(hashtable, key);
  pair = hashtable->table[bin];
  last = pair;

  while (pair != NULL && strcmp(key, pair->key) != 0) {
    last = pair;
    pair = pair->next;
  }

  if (pair != NULL && strcmp(key, pair->key) == 0) {
    // key was found
    if (pair ==  hashtable->table[bin]) { // only one key in the bucket
      free(pair->key);
      free(pair->value);

      hashtable->table[bin] = NULL;
      free(pair);
    } else {
      last->next = pair->next;

      free(pair->key);
      free(pair->value);
      free(pair);
    }
    ret = 0;
  }

  return ret;
}

int reprompib_dict_get_nkeys(const reprompib_dictionary_t* hashtable) {
  int i;
  int nkeys = 0;

  for (i = 0; i < hashtable->size; i++) {
    if (hashtable->table[i] != NULL) {
      entry_t *cur;
      cur = hashtable->table[i];
      while (cur != NULL) {
        nkeys++;
        cur = cur->next;
      }
    }
  }

  return nkeys;
}

int reprompib_get_keys_from_dict(const reprompib_dictionary_t* hashtable, char ***keys, int *nkeys) {
  int i, key_idx;
  int ret = 1;

  *nkeys = reprompib_dict_get_nkeys(hashtable);
  *keys = NULL;

  if (*nkeys > 0) {
    *keys = (char**) calloc(*nkeys, sizeof(char*));
    if (*keys == NULL) {
      return ret;
    }

    key_idx = 0;
    for (i = 0; i < hashtable->size; i++) {
      if (hashtable->table[i] != NULL) {
        entry_t *cur;
        cur = hashtable->table[i];
        while (cur != NULL) {
          (*keys)[key_idx] = strdup(cur->key);
          key_idx++;
          cur = cur->next;
        }
      }
    }
  }

  return 0;
}

int reprompib_dict_is_empty(const reprompib_dictionary_t* hashtable) {
  int nkeys = reprompib_dict_get_nkeys(hashtable);
  return (nkeys == 0);
}

int reprompib_dict_has_key(const reprompib_dictionary_t* hashtable, const char *key) {
  int ret;
  char *value;
  int found_key = 0;

  ret = reprompib_get_value_from_dict(hashtable, key, &value);
  if (ret == 0) {
    found_key = 1;
  }
  return found_key;
}

void reprompib_print_dictionary(const reprompib_dictionary_t* hashtable, FILE* f) {
  int i;
  int nkeys;

  nkeys = reprompib_dict_get_nkeys(hashtable);

  if (nkeys > 0) {
    fprintf(f, "#Key-value parameters:\n");
    for (i = 0; i < hashtable->size; i++) {
      if (hashtable->table[i] != NULL) {
        entry_t *cur;
        cur = hashtable->table[i];
        while (cur != NULL) {
          fprintf(f, "#@%s=%s\n", cur->key, cur->value);
          cur = cur->next;
        }
      }
    }
    fprintf(f, "# \n");
  }
}

