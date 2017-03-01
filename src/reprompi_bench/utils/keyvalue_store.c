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

static const int LEN_KEYVALUE_LIST_BATCH = 4;


//static reprompib_dictionary_t dict;

void reprompib_init_dictionary(reprompib_dictionary_t* dict) {

    dict->size = LEN_KEYVALUE_LIST_BATCH;
    dict->n_elems = 0;
    dict->data = (reprompib_dict_keyval_t *) malloc(LEN_KEYVALUE_LIST_BATCH * sizeof(reprompib_dict_keyval_t));
}


void reprompib_cleanup_dictionary(reprompib_dictionary_t* dict) {
    if (dict->data != NULL) {
        int i;
        for (i =0; i<dict->n_elems; i++) {
            free(dict->data[i].key);
            free(dict->data[i].value);
        }
        free(dict->data);
    }
    dict->size = 0;
    dict->n_elems = 0;
}


reprompib_dict_error_t reprompib_add_element_to_dict(reprompib_dictionary_t* dict, const char* key, const char* val) {
    reprompib_dict_error_t ok = DICT_SUCCESS;
    int i;

    if (key!=NULL && val!= NULL) {
       for (i=0; i<dict->n_elems; i++) {
           if (strcmp(key, dict->data[i].key) == 0) {   // key already exists - replace value with the new one
               free(dict->data[i].value);
               dict->data[i].value = strdup(val);
               break;
           }
       }

       if (i == dict->n_elems) { // add new key
           dict->data[dict->n_elems].key = strdup(key);
           dict->data[dict->n_elems].value = strdup(val);
           dict->n_elems++;
       }
    }
    else {
        if (key == NULL) {
            ok = DICT_ERROR_NULL_KEY;
        }
        else {
            ok = DICT_ERROR_NULL_VALUE;
        }
    }

    if (dict->n_elems == dict->size) {
        dict->size += LEN_KEYVALUE_LIST_BATCH;
        dict->data = (reprompib_dict_keyval_t*) realloc(dict->data, dict->size * sizeof(reprompib_dict_keyval_t));
    }
    return ok;
}


char* reprompib_get_value_from_dict(const reprompib_dictionary_t* dict, const char* key) {
    char* val = NULL;
    int i = 0;

    if (key == NULL) {
        val = NULL;
    }
    else {
        for (i=0; i<dict->n_elems; i++) {
            if (strcmp(key, dict->data[i].key) == 0) {
                val = strdup(dict->data[i].value);
                break;
            }
        }
    }
    return val;
}


reprompib_dict_error_t reprompib_remove_element_from_dict(reprompib_dictionary_t* dict, const char* key) {
  reprompib_dict_error_t ok = DICT_SUCCESS;
    int i = 0, j;

    if (key == NULL) {
        ok = DICT_ERROR_NULL_KEY;
    }
    else {
        ok = DICT_KEY_ERROR;
        for (i=0; i<dict->n_elems; i++) {
            if (strcmp(key, dict->data[i].key) == 0) {
                free(dict->data[i].key);
                free(dict->data[i].value);

                for (j=i; j< dict->n_elems-1; j++) {
                    dict->data[j].key = dict->data[j+1].key;
                    dict->data[j].value = dict->data[j+1].value;
                }

                dict->n_elems = dict->n_elems-1;
                ok = DICT_SUCCESS;
                break;
            }
        }
    }

    return ok;
}



reprompib_dict_error_t reprompib_get_keys_from_dict(const reprompib_dictionary_t* dict, char ***keys, int *length) {
  int i;
  *length = dict->n_elems;
  *keys   = (char**)calloc(dict->n_elems, sizeof(char*));
  for(i=0; i<dict->n_elems; i++) {
    (*keys)[i] = strdup(dict->data[i].key);
  }

  return DICT_SUCCESS;
}


int reprompib_dict_is_empty(const reprompib_dictionary_t* dict) {
  return (dict->n_elems == 0);
}

int reprompib_dict_get_length(const reprompib_dictionary_t* dict) {
  return dict->n_elems;
}


int reprompib_dict_has_key(const reprompib_dictionary_t* dict, const char *key) {
  int i;
  int found_key = 0;

  for(i=0; i<dict->n_elems; i++) {
    if (strcmp(dict->data[i].key, key) == 0) {
      found_key = 1;
      break;
    }
  }
  return found_key;
}


void reprompib_print_dictionary(const reprompib_dictionary_t* dict, FILE* f) {
    int i = 0;

    if (dict->n_elems > 0) {
        fprintf(f, "#Key-value parameters:\n");
        for (i = 0; i < dict->n_elems; i++) {
            fprintf(f, "#@%s=%s\n", dict->data[i].key, dict->data[i].value);
        }
        fprintf(f, "# \n");
    }
}


