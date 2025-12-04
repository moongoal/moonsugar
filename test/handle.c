#include <moondance/test.h>
#include <moonsugar/api.h>

MS_HDECL(test_handle);

MD_CASE(hcmp) {
    test_handle h10 = {1}, h2 = {2}, h11 = {1};

    md_assert(ms_hcmp(h10, h11) == 0);
    md_assert(ms_hcmp(h10, h2) > 0);
    md_assert(ms_hcmp(h2, h10) > 0);
}

MD_CASE(heq) {
    test_handle h10 = {1}, h2 = {2}, h11 = {1};

    md_assert(ms_heq(h10, h11) == true);
    md_assert(ms_heq(h10, h2) == false);
}

MD_CASE(is_valid) {
    test_handle hvalid = {0}, hinvalid = {MS_HINVALID};

    md_assert(ms_his_valid(hvalid) == true);
    md_assert(ms_his_valid(hinvalid) == false);
}

int main(int argc, char **argv) {
    md_suite suite = md_suite_create();

    md_add(&suite, hcmp);
    md_add(&suite, heq);
    md_add(&suite, is_valid);

    return md_run(argc, argv, &suite);
}

