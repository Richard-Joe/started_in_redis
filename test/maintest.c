#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "testcases.h"


int init_suite(void) {
    return 0;
}


int clean_suite(void) {
    return 0;
}


int main(int argc, char*argv[]) {
   CU_pSuite pSuite = NULL;

   if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    pSuite = CU_add_suite("Suite", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_add_test(pSuite, "test of sds", sdsTest);
    CU_add_test(pSuite, "test of dlist", dlistTest);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}