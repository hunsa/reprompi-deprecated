/*  ReproMPI Benchmark
 *
 *  Copyright (c) 2021 Stefan Christians
 *
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

#ifndef ICMB_LAUNCHERS_H
#define ICMB_LAUNCHERS_H


void icmb_launch_client(char* port_name);
void icmb_launch_master(int num_workers, char* command, char** argv);
void icmb_launch_server();
void icmb_launch_split(int num_workers);
void icmb_launch_standard();
void icmb_launch_worker();

#endif /* ICMB_LAUNCHERS_H */
