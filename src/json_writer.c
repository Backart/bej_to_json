/**
 * @file json_writer.c  
 * @brief JSON writer implementation - contains function definitions
 */

#include "bej_parser.h"
#include "json_writer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

struct dynamic_string* dynamic_string_init() {
    struct dynamic_string *str = malloc(sizeof(struct dynamic_string));
    str->capacity = 1024;
    str->length = 0;
    str->data = malloc(str->capacity);
    str->data[0] = '\0';
    return str;
}

void dynamic_string_append(struct dynamic_string *str, const char *s) {
    size_t slen = strlen(s);
    if (str->length + slen + 1 >= str->capacity) {
        while (str->length + slen + 1 >= str->capacity) str->capacity *= 2;
        str->data = realloc(str->data, str->capacity);
    }
    memcpy(str->data + str->length, s, slen);
    str->length += slen;
    str->data[str->length] = '\0';
}

void parse_bej_node_to_str_recursion(struct bej_node *node, struct dynamic_string *str, 
                                     const char *key, int indent,
                                     struct field_map *map, size_t map_count) {
    if (!node) return;

    for (int i = 0; i < indent; i++) dynamic_string_append(str, "  ");

    if (key) {
        dynamic_string_append(str, "\"");
        dynamic_string_append(str, key);
        dynamic_string_append(str, "\": ");
    }

    switch(node->format) {
        case 0: // BEJ_FORMAT_NULL
            dynamic_string_append(str, "null");
            break;
            
        case 5: // BEJ_FORMAT_STRING
            dynamic_string_append(str, "\"");
            dynamic_string_append(str, node->value ? (char*)node->value : "");
            dynamic_string_append(str, "\"");
            break;

        case 3: // BEJ_FORMAT_INTEGER
            if (node->value) {
                char buf[32];
                if (node->length == 1) 
                    snprintf(buf, sizeof(buf), "%d", *(int8_t*)node->value);
                else if (node->length == 2)
                    snprintf(buf, sizeof(buf), "%d", *(int16_t*)node->value);
                else if (node->length == 4)
                    snprintf(buf, sizeof(buf), "%d", *(int32_t*)node->value);
                else
                    snprintf(buf, sizeof(buf), "%" PRId64, *(int64_t*)node->value);
                dynamic_string_append(str, buf);
            } else {
                dynamic_string_append(str, "0");
            }
            break;

        case 6: // BEJ_FORMAT_BOOLEAN
            dynamic_string_append(str, node->value && *(int*)node->value ? "true" : "false");
            break;

        case 1: // BEJ_FORMAT_SET
        case 2: // BEJ_FORMAT_ARRAY
            dynamic_string_append(str, node->format == 1 ? "{\n" : "[\n");

            for (size_t i = 0; i < node->children_count; i++) {
                char child_key[64] = {0};

                if (node->format == 1) { // SET
                    const char *name = get_field_name(node->children[i]->sequence, map, map_count);
                    if (!name) {
                        snprintf(child_key, sizeof(child_key), "field_%" PRIu64, node->children[i]->sequence);
                        name = child_key;
                    }
                    parse_bej_node_to_str_recursion(node->children[i], str, name, indent + 1, map, map_count);
                } else { // ARRAY
                    parse_bej_node_to_str_recursion(node->children[i], str, NULL, indent + 1, map, map_count);
                }

                if (i + 1 < node->children_count) dynamic_string_append(str, ",\n");
                else dynamic_string_append(str, "\n");
            }

            for (int i = 0; i < indent; i++) dynamic_string_append(str, "  ");
            dynamic_string_append(str, node->format == 1 ? "}" : "]");
            break;

        case 4: // BEJ_FORMAT_ENUM
            if (node->value) {
                char buf[32];
                snprintf(buf, sizeof(buf), "%" PRIu64, *(uint64_t*)node->value);
                dynamic_string_append(str, buf);
            }
            break;

        default:
            dynamic_string_append(str, "\"<unknown>\"");
            break;
    }
}