#ifndef _READ_CSV_H_ 
#define _READ_CSV_H_

#include "utils.h"
#include "cn_profile.h"
#include <stdio.h>
#include <stdbool.h>

cn_profile_data_t* read_cn_profiles(char* filepath);
bool verify_headers(FILE* csv_file);

#endif // _READ_CSV_H_