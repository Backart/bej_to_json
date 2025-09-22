/**
 * @file bej_parser.h
 * @brief BEJ (Binary-encoded JSON) parser structures and functions
 */

#ifndef BEJ_PARSER_H
#define BEJ_PARSER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

/** BEJ format types according to DSP0218 specification */
#define BEJ_FORMAT_SET       0x01    /**< Set type */
#define BEJ_FORMAT_ARRAY     0x02    /**< Array type */  
#define BEJ_FORMAT_INTEGER   0x03    /**< Integer type */
#define BEJ_FORMAT_ENUM      0x04    /**< Enumeration type */
#define BEJ_FORMAT_STRING    0x05    /**< String type */
#define BEJ_FORMAT_BOOLEAN   0x06    /**< Boolean type */
#define BEJ_FORMAT_REAL      0x07    /**< Real number type */
#define BEJ_FORMAT_PROPERTY  0x08    /**< Property type */
#define BEJ_FORMAT_NULL      0x00    /**< Null type */

/** BEJ node structure representing parsed data element */
struct bej_node {
    uint8_t format;              /**< BEJ format type (bits 0-3) */
    uint8_t format_flags;        /**< Format flags (bits 4-7) */
    size_t length;               /**< Data length in bytes */
    void *value;                 /**< Pointer to node value */
    struct bej_node **children;  /**< Array of child nodes */
    size_t children_count;       /**< Number of child nodes */
    uint64_t sequence;           /**< Dictionary sequence number */
    uint8_t dictionary_type;     /**< Dictionary type (0=main, 1=annotation) */
};

/** Field mapping structure for sequence number to name mapping */
struct field_map {
    uint64_t sequence;           /**< Field sequence number */
    char *name;                  /**< Field name string */
};

/**
 * @brief Load field map from file
 * @param map_path Path to map file
 * @param count Output parameter for number of loaded entries
 * @return Pointer to allocated field map array
 */
struct field_map* load_map(const char *map_path, size_t *count);

/**
 * @brief Free memory allocated for field map
 * @param map Field map array to free
 * @param count Number of entries in the map
 */
void free_map(struct field_map *map, size_t count);

/**
 * @brief Get field name by sequence number
 * @param seq Sequence number to look up
 * @param map Field map array
 * @param count Number of entries in the map
 * @return Field name or NULL if not found
 */
const char* get_field_name(uint64_t seq, struct field_map *map, size_t count);

/**
 * @brief Initialize BEJ parsing from binary data
 * @param bej Pointer to BEJ binary data
 * @param bej_len Length of BEJ data in bytes
 * @param schema_dict Schema dictionary (optional)
 * @return Root BEJ node or NULL on error
 */
struct bej_node* parse_sflv_init(unsigned char *bej, size_t bej_len, unsigned char *schema_dict);

/**
 * @brief Recursive BEJ parsing function
 * @param node Current node being parsed
 * @param data Pointer to current position in data buffer
 * @param data_end Pointer to end of data buffer
 * @param schema_dict Schema dictionary (optional)
 * @param buffer_start Pointer to start of data buffer
 */
void parse_sflv_recursion(struct bej_node *node, unsigned char **data, unsigned char *data_end, unsigned char *schema_dict, unsigned char *buffer_start);

/**
 * @brief Free memory allocated for BEJ node tree
 * @param node Root node to free
 */
void free_bej_node(struct bej_node *node);

/** Basic data reading functions */
uint64_t read_uint64(unsigned char **data, int length, unsigned char *data_end);
int read_int(unsigned char **data, int length, unsigned char *data_end);
char* read_str(unsigned char **data, int length, unsigned char *data_end);
uint64_t read_varint_u64(unsigned char **data, unsigned char *data_end);

#endif // BEJ_PARSER_H