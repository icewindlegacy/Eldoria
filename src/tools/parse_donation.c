#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv)
{
    const char *file = "test_donation.dat";
    if (argc > 1) file = argv[1];

    FILE *fp = fopen(file, "r");
    if (!fp) { perror("fopen"); return 1; }

    int lines = 0;
    int objects = 0;
    while (1)
    {
        int c = fgetc(fp);
        if (c == EOF) { printf("EOF reached after %d lines, objects=%d\n", lines, objects); break; }
        if (c == '*') { /* skip comment */
            while ((c = fgetc(fp)) != EOF && c != '\n');
            lines++; continue;
        }
        if (!isspace(c)) {
            if (c != '#') {
                ungetc(c, fp);
                char word[256];
                if (fscanf(fp, "%255s", word) == 1) {
                    if (strcmp(word, "END") == 0) { printf("Found END after %d lines, objects=%d\n", lines, objects); break; }
                    printf("Skipping non-# word '%s'\n", word);
                    /* skip rest of line */
                    while ((c = fgetc(fp)) != EOF && c != '\n');
                    lines++; continue;
                } else { printf("Couldn't read a word at line %d\n", lines); break; }
            }
            /* at '#', read next word */
            char word[256];
            if (fscanf(fp, " %255s", word) != 1) { printf("Couldn't read word after #\n"); break; }
            if (strcmp(word, "O") == 0) {
                /* expect Vnum next or Vnum on next line */
                char next[256];
                if (fscanf(fp, " %255s", next) != 1) { printf("No token after O\n"); break; }
                if (strcmp(next, "Vnum") == 0) {
                    int vnum = 0; if (fscanf(fp, " %d", &vnum) == 1) {
                        printf("Found object vnum=%d\n", vnum);
                        objects++;
                    } else { printf("No vnum after Vnum\n"); }
                } else if (isdigit((unsigned char)next[0]) ) {
                    int vnum = atoi(next);
                    printf("Found object vnum=%d\n", vnum);
                    objects++;
                } else {
                    printf("Unexpected token after #O: '%s'\n", next);
                }
            } else if (strcmp(word, "END") == 0) {
                printf("Found END after %d lines, objects=%d\n", lines, objects); break;
            } else {
                printf("Unknown # token '%s'\n", word);
            }
        }
        if (c == '\n') lines++;
    }
    fclose(fp);
    return 0;
}
