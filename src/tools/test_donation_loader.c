#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stubs to call fread_obj_donation */

FILE *fp;

char fread_letter(FILE *f);
char *fread_word(FILE *f);
char *fread_string(FILE *f);
int fread_number(FILE *f);
char *fread_string_eol(FILE *f);
void fread_to_eol(FILE *f);

/* We'll declare the function from save.c */
#include "../save.c"

int main(int argc, char **argv)
{
    const char *testfile = "test_donation.dat";
    if (argc > 1) testfile = argv[1];
    fp = fopen(testfile, "r");
    if (!fp) { perror("fopen"); return 1; }

    OBJ_DATA *obj = fread_obj_donation(fp);
    if (!obj) {
        printf("fread_obj_donation returned NULL\n");
    } else {
        printf("fread_obj_donation returned object: short='%s' vnum=%d\n",
            obj->short_descr ? obj->short_descr : "(null)", obj->pIndexData ? obj->pIndexData->vnum : -1);
    }
    fclose(fp);
    return 0;
}
