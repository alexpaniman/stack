#include "test-framework.h"
#include "printf-asm.h"

bool test_itoa(int base, int number, const char* number_string) {
    // Definitely enough for any number in any base
    const size_t buffer_size = 128;

    setvbuf(stdout, NULL, _IONBF, 0);

    char string[buffer_size] = {};
    stringify_integer(string, number, base);

    if (strncasecmp(string, number_string, buffer_size) != 0)
        return false;

    return true;
}

bool compare_itoa_output_with_printf(const char* printf_number_format, int base, int number) {
    // Definitely enough for any number in any base
    const size_t buffer_size = 128;

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wformat-nonliteral"

    char reference_number[buffer_size];
    snprintf(reference_number, buffer_size, printf_number_format, number);

    #pragma clang diagnostic pop

    return test_itoa(base, number, reference_number);
}

struct itoa_test {
    int base, number;
    const char* number_string;
};

TEST(test_itoa_decimal) {
    const int test_up_to = 100000;
    for (int i = 0; i < test_up_to; ++ i)
        ASSERT_EQUAL(compare_itoa_output_with_printf("%d", 10, i), true);
}

TEST(test_itoa_hex) {
    const int test_up_to = 100000;
    for (int i = 0; i < test_up_to; ++ i)
        ASSERT_EQUAL(compare_itoa_output_with_printf("%x", 16, i), true);
}

TEST(test_itoa_octal) {
    const int test_up_to = 100000;
    for (int i = 0; i < test_up_to; ++ i)
        ASSERT_EQUAL(compare_itoa_output_with_printf("%o", 8, i), true);
}

TEST(test_printf) {
    asm_printf("Test string:  %s\n", "string");
    asm_printf("Test decimal: %d\n", 10);
    asm_printf("Test octal:   %o\n", 0123);
    asm_printf("Test binary:  %b\n", 15);
    asm_printf("Test char:    %c\n", 'x');
    asm_printf("Test percent: %%\n", 'x');

    asm_printf("\n==> Complex: \n");

    asm_printf("%f%%\n");
    asm_printf("%d == hello %s \n", 100, "string");
    asm_printf("%s%x & %d%%%c%b\n", "something", 0xDED, 100, '!', 15);
}

int main(void) {
    return test_framework_run_all_unit_tests();
}
