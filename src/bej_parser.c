/**
 * @file bej_parser.c
 * @brief BEJ parser implementation - contains function definitions
 */

#define _POSIX_C_SOURCE 200809L
#include "bej_parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>


struct field_map* load_map(const char *map_path, size_t *count) {
    if (!map_path || !count) return NULL;

    FILE *f = fopen(map_path, "r");
    if (!f) return NULL;

    struct field_map *map = NULL;
    *count = 0;
    char line[256];

    while (fgets(line, sizeof(line), f)) {
        char *colon = strchr(line, ':');
        if (!colon) continue;
        *colon = '\0';

        uint64_t seq = strtoull(line, NULL, 10);
        char *name = colon + 1;

        char *newline = strchr(name, '\n');
        if (newline) *newline = '\0';

        struct field_map *tmp = realloc(map, (*count + 1) * sizeof(struct field_map));
        if (!tmp) { perror("realloc"); free_map(map, *count); fclose(f); return NULL; }
        map = tmp;

        map[*count].sequence = seq;
        map[*count].name = strdup(name);
        (*count)++;
    }

    fclose(f);
    return map;
}

void free_map(struct field_map *map, size_t count) {
    if (!map) return;
    for (size_t i = 0; i < count; i++) free(map[i].name);
    free(map);
}

const char* get_field_name(uint64_t seq, struct field_map *map, size_t count) {
    if (!map) return NULL;
    for (size_t i = 0; i < count; i++)
        if (map[i].sequence == seq) 
            return map[i].name;
    return NULL;
}

bool read_varint_safe(unsigned char **data, unsigned char *data_end, uint64_t *out) {
    if (!data || !*data || !out) return false;

    uint64_t value = 0;
    int shift = 0;

    while (*data < data_end) {
        unsigned char byte = **data;
        (*data)++;

        if (shift >= 64 && (byte & 0x7F)) return false;
        value |= (uint64_t)(byte & 0x7F) << shift;

        if ((byte & 0x80) == 0) {
            *out = value;
            return true;
        }

        shift += 7;
    }

    return false;
}

uint64_t read_varint_u64(unsigned char **data, unsigned char *data_end) {
    uint64_t val = 0;
    if (!read_varint_safe(data, data_end, &val)) return 0;
    return val;
}

uint64_t read_uint64(unsigned char **data, int length, unsigned char *data_end) {
    if (*data + length > data_end) return 0;
    uint64_t val = 0;
    for (int i = 0; i < length; i++) {
        val = (val << 8) | (*(*data + i));
    }
    *data += length;
    return val;
}

int read_int(unsigned char **data, int length, unsigned char *data_end) {
    if (*data + length > data_end) return 0;
    int val = 0;
    for (int i = 0; i < length; i++) {
        val = (val << 8) | (*(*data + i));
    }
    *data += length;
    return val;
}

char* read_str(unsigned char **data, int length, unsigned char *data_end) {
    if (*data + length > data_end) return NULL;
    char *str = malloc(length + 1);
    memcpy(str, *data, length);
    str[length] = '\0';
    *data += length;
    return str;
}

void parse_sflv_recursion(struct bej_node *node, unsigned char **data, unsigned char *data_end, unsigned char *schema_dict, unsigned char *buffer_start) {
    if (!node || *data >= data_end) return;

    uint64_t seq = read_varint_u64(data, data_end);
    node->dictionary_type = seq & 1;
    node->sequence = seq >> 1;

    if (*data >= data_end) return;

    uint8_t format_byte = **data;
    node->format = format_byte & 0x0F;
    node->format_flags = (format_byte >> 4);
    (*data)++;

    bool is_null = (format_byte & 0x10) != 0;
    bool is_readonly = (format_byte & 0x20) != 0;
    uint8_t selector = (format_byte >> 6) & 0x03;

    printf("DEBUG: null=%d, readonly=%d, selector=%u\n", is_null, is_readonly, selector);

    node->length = read_varint_u64(data, data_end);

    node->children_count = 0;
    node->children = NULL;
    node->value = NULL;

    printf("DEBUG: seq=%" PRIu64 ", dict_type=%u, format_byte=0x%02X, format=%u, length=%zu\n",
           node->sequence, node->dictionary_type, format_byte, node->format, node->length);

    switch (node->format) {
        case 0:
            *data += node->length;
            break;
            
        case 3: // BEJ_FORMAT_INTEGER
            if (node->length == 1) {
                int8_t *val = malloc(sizeof(int8_t));
                *val = (int8_t)(**data);
                (*data)++;
                node->value = val;
            } else if (node->length == 2) {
                int16_t *val = malloc(sizeof(int16_t));
                *val = (int16_t)read_uint64(data, 2, data_end);
                node->value = val;
            } else if (node->length == 4) {
                int32_t *val = malloc(sizeof(int32_t));
                *val = (int32_t)read_uint64(data, 4, data_end);
                node->value = val;
            } else {
                int64_t *val = malloc(sizeof(int64_t));
                *val = (int64_t)read_uint64(data, node->length, data_end);
                node->value = val;
            }
            break;
            
        case 6: // BEJ_FORMAT_BOOLEAN
            if (*data < data_end) {
                int *val = malloc(sizeof(int));
                *val = **data ? 1 : 0;
                (*data)++;
                node->value = val;
            }
            break;
            
        case 5: // BEJ_FORMAT_STRING
            node->value = read_str(data, node->length, data_end);
            break;
            
        case 4: // BEJ_FORMAT_ENUM
            {
                uint64_t *val = malloc(sizeof(uint64_t));
                *val = read_varint_u64(data, data_end);
                node->value = val;
            }
            break;
            
        case 1: // BEJ_FORMAT_SET
        case 2: // BEJ_FORMAT_ARRAY
            {
                unsigned char *end_ptr = *data + node->length;
                while (*data < end_ptr && *data < data_end) {
                    node->children_count++;
                    struct bej_node **tmp = realloc(node->children, node->children_count * sizeof(struct bej_node*));
                    if (!tmp) { perror("realloc"); exit(1); }
                    node->children = tmp;

                    node->children[node->children_count - 1] = calloc(1, sizeof(struct bej_node));
                    parse_sflv_recursion(node->children[node->children_count - 1], data, end_ptr, schema_dict, buffer_start);
                }
            }
            break;
            
        default:
            *data += node->length;
            break;
    }
}

struct bej_node* parse_sflv_init(unsigned char *data, size_t data_len, unsigned char *schema_dict) {
    if (!data || data_len == 0) return NULL;

    struct bej_node *root = calloc(1, sizeof(struct bej_node));
    root->format = BEJ_FORMAT_SET;
    root->sequence = 0;
    root->dictionary_type = 0;
    root->children_count = 0;
    root->children = NULL;

    unsigned char *ptr = data;
    unsigned char *end = data + data_len;

    while (ptr < end) {
        root->children_count++;
        struct bej_node **tmp = realloc(root->children, root->children_count * sizeof(struct bej_node*));
        if (!tmp) { perror("realloc"); exit(1); }
        root->children = tmp;

        root->children[root->children_count - 1] = calloc(1, sizeof(struct bej_node));
        parse_sflv_recursion(root->children[root->children_count - 1], &ptr, end, schema_dict, data);
    }

    return root;
}

void free_bej_node(struct bej_node *node) {
    if (!node) return;
    for (size_t i = 0; i < node->children_count; i++)
        free_bej_node(node->children[i]);
    free(node->children);
    free(node->value);
    free(node);
}