/* Inter-Communicator MPI Benchmarking
 * Copyright (c) 2021 Stefan Christians
 * SPDX-License-Identifier: MIT
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
