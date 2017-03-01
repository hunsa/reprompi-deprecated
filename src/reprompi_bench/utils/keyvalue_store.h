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

//#include "reprompi_bench/misc.h"

typedef struct reprompib_params_keyval {
    char* key;
    char* value;
} reprompib_params_keyval_t;


typedef struct reprompib_dictionary {
    reprompib_params_keyval_t* data;
    int n_elems;
    int size;
} reprompib_dictionary_t;

typedef enum {
    DICT_SUCCESS = 0,
    DICT_ERROR_KEY_VAL_PARAM
} reprompib_dict_error_t;



void reprompib_init_dictionary(void);
void reprompib_cleanup_dictionary(void);
reprompib_dict_error_t reprompib_add_element_to_dict(char* key, const char* val);
char* reprompib_get_value_from_dict(char* key);
reprompib_dict_error_t reprompib_remove_element_from_dict(char* key);

void reprompib_print_dictionary(FILE* f);


#endif /* REPROMPIB_KEYVALUE_STORE_H_ */
