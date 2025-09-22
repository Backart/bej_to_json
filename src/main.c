/**
 * @file main.c
 * @brief BEJ to JSON converter - main application entry point
 */

#include "bej_parser.h"
#include "json_writer.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Main function - converts BEJ file to JSON using map file
 * @param argc Number of command line arguments
 * @param argv Command line arguments: [program] <bej_file> <map_file>
 * @return 0 on success, 1 on error
 * 
 * @usage ./bej_to_json input.bej dictionary.map
 */
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <bej_file> <map_file>\n", argv[0]);
        return 1;
    }

    // Read BEJ file into memory
    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror("Cannot open BEJ file"); return 1; }
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    unsigned char *buf = malloc(size);
    if (!buf) { perror("Memory allocation failed"); fclose(f); return 1; }
    if (fread(buf, 1, size, f) != size) { perror("Reading BEJ file failed"); free(buf); fclose(f); return 1; }
    fclose(f);

    // Load field map
    size_t map_count;
    struct field_map *map_array = load_map(argv[2], &map_count);
    if (!map_array) { fprintf(stderr, "Failed to load map\n"); free(buf); return 1; }

    // Parse BEJ and convert to JSON
    struct bej_node *root = parse_sflv_init(buf, size, NULL);
    if (!root) { fprintf(stderr, "BEJ parsing failed\n"); free(buf); free_map(map_array, map_count); return 1; }

    struct dynamic_string *json_str = dynamic_string_init();
    parse_bej_node_to_str_recursion(root, json_str, NULL, 0, map_array, map_count);

    // Output JSON result
    printf("%s\n", json_str->data);

    // Cleanup
    free_bej_node(root);
    free(json_str->data);
    free(json_str);
    free(buf);
    free_map(map_array, map_count);

    return 0;
}