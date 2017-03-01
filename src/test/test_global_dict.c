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
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "../reprompi_bench/misc.h"
#include "../reprompi_bench/utils/keyvalue_store.h"

int main(int argc, char* argv[]) {
    int i;
    int n =10;
    int result;
    char* s;
    char buf[100];
    reprompib_dictionary_t params_dict;

    reprompib_init_dictionary(&params_dict);
    for (i=0; i<n ;i++) {
        sprintf(buf, "%d", i);
        reprompib_add_element_to_dict(&params_dict, buf, buf);
    }

    reprompib_print_dictionary(&params_dict, stdout);

    printf("Trying to add same key twice...");
    i=2;
    sprintf(buf, "%d", i);
    result = reprompib_add_element_to_dict(&params_dict, buf, "new value");
    if (result != 0) {
        strcpy(buf, "error");
    }
    else {
        strcpy(buf,"success");
    }
    printf("add (%d, \"new value\") -> %s \n", i, buf);

    printf("Retrieve value...\n");
    i=5;
    sprintf(buf, "%d", i);
    s = reprompib_get_value_from_dict(&params_dict, buf);
    printf("get_value(%d) -> %s \n", i, s);
    free(s);

    i=10;
    sprintf(buf, "%d", i);
    s = reprompib_get_value_from_dict(&params_dict, buf);
    printf("get_value(%d) -> %s \n", i, s);
    free(s);

    printf("Remove element...");
    i=2;
    sprintf(buf, "%d", i);
    result = reprompib_remove_element_from_dict(&params_dict, buf);
    if (result != 0) {
        strcpy(buf, "error");
    }
    else {
        strcpy(buf,"success");
    }
    printf("remove (%d) -> %s \n", i, buf);

    printf("\n");
    reprompib_print_dictionary(&params_dict, stdout);

    printf("Remove everything...\n");
    for (i=0; i<n ;i++) {
        sprintf(buf, "%d", i);
        result = reprompib_remove_element_from_dict(&params_dict, buf);
        if (result != 0) {
            strcpy(buf, "error");
        }
        else {
            strcpy(buf,"success");
        }
        printf("remove (%d) -> %s \n", i, buf);
    }

    printf("Add elements again...\n");
    for (i=0; i<2* n ;i++) {
        sprintf(buf, "%d", i);
        reprompib_add_element_to_dict(&params_dict, buf, buf);
    }

    reprompib_print_dictionary(&params_dict, stdout);

    reprompib_cleanup_dictionary(&params_dict);

    return 0;
}
