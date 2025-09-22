/**
 * @file json_writer.h  
 * @brief JSON writer utilities for BEJ to JSON conversion
 */

#ifndef JSON_WRITER_H
#define JSON_WRITER_H

#include "bej_parser.h"
#include <stddef.h>

// Forward declaration
struct field_map;

/** Dynamic string structure for building JSON output */
struct dynamic_string {
    char *data;        /**< String data buffer */
    size_t length;     /**< Current string length */
    size_t capacity;   /**< Buffer capacity */
};

/** Map entry structure (unused in current implementation) */
struct map_entry {
    char *name;                    /**< Entry name */
    struct map_entry **children;   /**< Child entries */
    size_t children_count;         /**< Number of children */
};

/**
 * @brief Initialize dynamic string
 * @return New dynamic string instance
 */
struct dynamic_string* dynamic_string_init();

/**
 * @brief Append string to dynamic string
 * @param str Dynamic string to append to
 * @param s String to append
 */
void dynamic_string_append(struct dynamic_string *str, const char *s);

/**
 * @brief Add indentation tabs to string
 * @param str Dynamic string
 * @param tab Number of tabs to add
 */
void add_tab(struct dynamic_string *str, int tab);

/**
 * @brief Convert BEJ node to JSON string recursively
 * @param node BEJ node to convert
 * @param str Dynamic string for JSON output
 * @param key Field key name (NULL for arrays)
 * @param indent Indentation level
 * @param map Field map for sequence to name conversion
 * @param map_count Number of entries in field map
 */
void parse_bej_node_to_str_recursion(struct bej_node *node, struct dynamic_string *str, 
                                     const char *key, int indent,
                                     struct field_map *map, size_t map_count);

// Note: parse_map_file and free_map_entry are not implemented
struct map_entry* parse_map_file(const char *filename);
void free_map_entry(struct map_entry *map);

#endif