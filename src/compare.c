#include "compare.h"
#include "read_csv.h"
#include "cn_profile.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        PRINT("Incorrect Usage: ./main [data.csv]");
    }

    LOG("[main] CSV_FILE: %s", argv[1]);
    cn_profile_data_t* data = read_cn_profiles(argv[1]);
    destroy_cp_profiles(data);

    return 0;
}