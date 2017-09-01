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

#ifndef REPROMPIB_KEYVALUE_STORE_H_
#define REPROMPIB_KEYVALUE_STORE_H_

struct entry_s {
  char* key;
  char* value;
  struct entry_s *next;
};

typedef struct entry_s entry_t;

struct hashtable_s {
  int size;
  struct entry_s **table;
};

typedef struct hashtable_s reprompib_dictionary_t;


int  reprompib_init_dictionary(reprompib_dictionary_t* hashtable, const int size);
void reprompib_cleanup_dictionary(reprompib_dictionary_t* hashtable);

int reprompib_add_element_to_dict(reprompib_dictionary_t* hashtable, const char* key, const char* val);
int reprompib_get_value_from_dict(const reprompib_dictionary_t* hashtable, const char* key, char** value);
int reprompib_remove_element_from_dict(reprompib_dictionary_t* hashtable, const char* key);
int reprompib_get_keys_from_dict(const reprompib_dictionary_t* hashtable, char ***keys, int *nkeys);
int reprompib_dict_is_empty(const reprompib_dictionary_t* hashtable);
int reprompib_dict_get_nkeys(const reprompib_dictionary_t* hashtable);
int reprompib_dict_has_key(const reprompib_dictionary_t* hashtable, const char *key);

void reprompib_print_dictionary(const reprompib_dictionary_t* hashtable, FILE* f);


#endif /* REPROMPIB_KEYVALUE_STORE_H_ */
