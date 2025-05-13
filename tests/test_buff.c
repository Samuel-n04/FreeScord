#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "../include/buffer.h"

#define ASSERT(condition, message)                                        \
    do                                                                    \
    {                                                                     \
        if (!(condition))                                                 \
        {                                                                 \
            fprintf(stderr, "FAILED: %s (line %d)\n", message, __LINE__); \
            return 0;                                                     \
        }                                                                 \
    } while (0)

int test_buff_create()
{
    // Test invalid fd
    buffer *b1 = buff_create(-1, 100);
    ASSERT(b1 == NULL, "buff_create should return NULL for invalid fd");

    // Test zero buffer size
    buffer *b2 = buff_create(STDIN_FILENO, 0);
    ASSERT(b2 == NULL, "buff_create should return NULL for zero buffer size");

    return 1;
}

int test_buff_getc_simple()
{
    // Create a temporary file with known content
    FILE *temp = tmpfile();
    ASSERT(temp != NULL, "Failed to create temporary file");

    const char *test_content = "Hello, World!";
    fputs(test_content, temp);
    fflush(temp);
    rewind(temp);

    // Get file descriptor
    int fd = fileno(temp);

    // Create buffer
    buffer *b = buff_create(fd, 5); // Small buffer to test refilling
    ASSERT(b != NULL, "Failed to create buffer");

    // Read characters one by one
    for (int i = 0; test_content[i]; i++)
    {
        int c = buff_getc(b);
        ASSERT(c == test_content[i], "Incorrect character read");
    }

    // Check EOF
    ASSERT(buff_getc(b) == EOF, "Should reach EOF");
    ASSERT(buff_eof(b), "EOF flag should be set");

    // Cleanup
    buff_free(b);
    fclose(temp);

    return 1;
}

int test_buff_ungetc()
{
    // Create a temporary file with known content
    FILE *temp = tmpfile();
    ASSERT(temp != NULL, "Failed to create temporary file");

    const char *test_content = "Test";
    fputs(test_content, temp);
    fflush(temp);
    rewind(temp);

    // Get file descriptor
    int fd = fileno(temp);

    // Create buffer
    buffer *b = buff_create(fd, 10);
    ASSERT(b != NULL, "Failed to create buffer");

    // Read first character
    int first_char = buff_getc(b);
    ASSERT(first_char == 'T', "First character should be 'T'");

    // Unget the character
    ASSERT(buff_ungetc(b, first_char) == first_char, "Ungetc should return the character");

    // Read it again
    ASSERT(buff_getc(b) == first_char, "Ungetc should allow reading the character again");

    // Try to unget a second character (should fail)
    ASSERT(buff_ungetc(b, 'X') == first_char, "First unget should succeed");
    ASSERT(buff_ungetc(b, 'Y') == EOF, "Second unget should fail");

    // Cleanup
    buff_free(b);
    fclose(temp);

    return 1;
}

int test_buff_fgets()
{
    // Create a temporary file with known content
    FILE *temp = tmpfile();
    ASSERT(temp != NULL, "Failed to create temporary file");

    const char *test_content = "Line 1\nLine 2\nLine 3\n";
    fputs(test_content, temp);
    fflush(temp);
    rewind(temp);

    // Get file descriptor
    int fd = fileno(temp);

    // Create buffer
    buffer *b = buff_create(fd, 5); // Small buffer to test refilling
    ASSERT(b != NULL, "Failed to create buffer");

    // Buffer to read lines
    char line[100];

    // Read first line
    ASSERT(buff_fgets(b, line, sizeof(line)) == line, "First fgets should succeed");
    ASSERT(strcmp(line, "Line 1\n") == 0, "First line should match");

    // Read second line
    ASSERT(buff_fgets(b, line, sizeof(line)) == line, "Second fgets should succeed");
    ASSERT(strcmp(line, "Line 2\n") == 0, "Second line should match");

    // Read third line
    ASSERT(buff_fgets(b, line, sizeof(line)) == line, "Third fgets should succeed");
    ASSERT(strcmp(line, "Line 3\n") == 0, "Third line should match");

    // Check EOF
    ASSERT(buff_fgets(b, line, sizeof(line)) == NULL, "Fourth fgets should return NULL");
    ASSERT(buff_eof(b), "EOF flag should be set");

    // Cleanup
    buff_free(b);
    fclose(temp);

    return 1;
}

int test_buff_fgets_crlf()
{
    // Create a temporary file with known content
    FILE *temp = tmpfile();
    ASSERT(temp != NULL, "Failed to create temporary file");

    const char *test_content = "Line 1\r\nLine 2\r\nLine 3\r\n";
    fputs(test_content, temp);
    fflush(temp);
    rewind(temp);

    // Get file descriptor
    int fd = fileno(temp);

    // Create buffer
    buffer *b = buff_create(fd, 5); // Small buffer to test refilling
    ASSERT(b != NULL, "Failed to create buffer");

    // Buffer to read lines
    char line[100];

    // Read first line
    ASSERT(buff_fgets_crlf(b, line, sizeof(line)) == line, "First fgets_crlf should succeed");
    ASSERT(strcmp(line, "Line 1\r\n") == 0, "First line should match");

    // Read second line
    ASSERT(buff_fgets_crlf(b, line, sizeof(line)) == line, "Second fgets_crlf should succeed");
    ASSERT(strcmp(line, "Line 2\r\n") == 0, "Second line should match");

    // Read third line
    ASSERT(buff_fgets_crlf(b, line, sizeof(line)) == line, "Third fgets_crlf should succeed");
    ASSERT(strcmp(line, "Line 3\r\n") == 0, "Third line should match");

    // Check EOF
    ASSERT(buff_fgets_crlf(b, line, sizeof(line)) == NULL, "Fourth fgets_crlf should return NULL");
    ASSERT(buff_eof(b), "EOF flag should be set");

    // Cleanup
    buff_free(b);
    fclose(temp);

    return 1;
}

int test_buff_ready()
{
    // Create a temporary file with known content
    FILE *temp = tmpfile();
    ASSERT(temp != NULL, "Failed to create temporary file");

    const char *test_content = "Test";
    fputs(test_content, temp);
    fflush(temp);
    rewind(temp);

    // Get file descriptor
    int fd = fileno(temp);

    // Create buffer
    buffer *b = buff_create(fd, 10);
    ASSERT(b != NULL, "Failed to create buffer");

    // Initially should not be ready
    ASSERT(!buff_ready(b), "Buffer should not be ready before reading");

    // Read first character
    int first_char = buff_getc(b);
    ASSERT(first_char == 'T', "First character should be 'T'");

    // Should be ready now
    ASSERT(buff_ready(b), "Buffer should be ready after reading");

    // Unget the character
    ASSERT(buff_ungetc(b, first_char) == first_char, "Ungetc should return the character");

    // Should still be ready
    ASSERT(buff_ready(b), "Buffer should be ready after unget");

    // Cleanup
    buff_free(b);
    fclose(temp);

    return 1;
}

int main()
{
    int total_tests = 5;
    int passed_tests = 0;

    printf("Running Buffer Tests:\n");

    passed_tests += test_buff_create();
    printf("test_buff_create: %s\n", passed_tests ? "PASSED" : "FAILED");

    passed_tests += test_buff_getc_simple();
    printf("test_buff_getc_simple: %s\n", passed_tests ? "PASSED" : "FAILED");

    passed_tests += test_buff_ungetc();
    printf("test_buff_ungetc: %s\n", passed_tests ? "PASSED" : "FAILED");

    passed_tests += test_buff_fgets();
    printf("test_buff_fgets: %s\n", passed_tests ? "PASSED" : "FAILED");

    passed_tests += test_buff_fgets_crlf();
    printf("test_buff_fgets_crlf: %s\n", passed_tests ? "PASSED" : "FAILED");

    passed_tests += test_buff_ready();
    printf("test_buff_ready: %s\n", passed_tests ? "PASSED" : "FAILED");

    printf("\nTest Summary: %d/%d tests passed\n",
           passed_tests, total_tests);

    return passed_tests == total_tests ? 0 : 1;
}