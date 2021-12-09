#include "include/lcp.test.h"
#include "include/lucretia.test.h"

#include <stdio.h>

static void run_test(int (*f)(void), int *success, int *fail);

int main(int argc, char const *argv[])
{
    int total_test_performed = 0;
    int total_test_success = 0;
    int total_test_failed = 0;

    run_test(to_str_lcp_req__given_lcp_req, &total_test_success, &total_test_success);
    total_test_performed++;

    run_test(serialize_lcp_req__given_lcp_req, &total_test_success, &total_test_success);
    total_test_performed++;

    run_test(deserialize_lcp_req__given_lcp_req, &total_test_success, &total_test_success);
    total_test_performed++;

    run_test(create_new_lucretia__given_propety_map, &total_test_success, &total_test_success);
    total_test_performed++;

    run_test(handshake__given_master_and_slave, &total_test_success, &total_test_success);
    total_test_performed++;

    fprintf(stdout, "\nTotal tests: %d Success: %d Fail: %d\n",
            total_test_performed, total_test_success, total_test_failed);
    return 0;
}

static void run_test(int (*f)(void), int *success, int *fail)
{
    int slr_result = f();

    if (slr_result)
        *success++;
    else
        *fail++;
}