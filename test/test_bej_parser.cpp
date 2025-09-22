#include <gtest/gtest.h>
#include "bej_wrapper.h"
#include <stdlib.h>
#include <string.h>

class BejParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_data = (unsigned char*)malloc(1024);
        ASSERT_TRUE(test_data != nullptr);
    }

    void TearDown() override {
        if (test_data) free(test_data);
    }

    unsigned char* test_data;
    size_t test_data_size;
};

// Тест для read_varint_u64
TEST_F(BejParserTest, ReadVarintBasic) {
    unsigned char data[] = {0x01};
    unsigned char* ptr = data;
    unsigned char* end = data + sizeof(data);
    
    uint64_t result = read_varint_u64(&ptr, end);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(ptr, data + 1);
}

TEST_F(BejParserTest, ReadVarintMultiByte) {
    unsigned char data[] = {0xAC, 0x02};
    unsigned char* ptr = data;
    unsigned char* end = data + sizeof(data);
    
    uint64_t result = read_varint_u64(&ptr, end);
    EXPECT_EQ(result, 300);
    EXPECT_EQ(ptr, data + 2);
}

// Тест для читання integer
TEST_F(BejParserTest, ReadInt8) {
    unsigned char data[] = {0x42};
    unsigned char* ptr = data;
    unsigned char* end = data + sizeof(data);
    
    int result = read_int(&ptr, 1, end);
    EXPECT_EQ(result, 0x42);
    EXPECT_EQ(ptr, data + 1);
}

TEST_F(BejParserTest, ReadInt32) {
    unsigned char data[] = {0x01, 0x02, 0x03, 0x04};
    unsigned char* ptr = data;
    unsigned char* end = data + sizeof(data);
    
    int result = read_int(&ptr, 4, end);
    EXPECT_EQ(result, 0x01020304);
    EXPECT_EQ(ptr, data + 4);
}

// Тест для читання string
TEST_F(BejParserTest, ReadString) {
    unsigned char data[] = {'H', 'e', 'l', 'l', 'o'};
    unsigned char* ptr = data;
    unsigned char* end = data + sizeof(data);
    
    char* result = read_str(&ptr, 5, end);
    ASSERT_TRUE(result != nullptr);
    EXPECT_STREQ(result, "Hello");
    EXPECT_EQ(ptr, data + 5);
    
    free(result);
}

TEST_F(BejParserTest, ReadStringNullTerminated) {
    unsigned char data[] = {'T', 'e', 's', 't', '\0'};
    unsigned char* ptr = data;
    unsigned char* end = data + sizeof(data);
    
    char* result = read_str(&ptr, 5, end);
    ASSERT_TRUE(result != nullptr);
    EXPECT_STREQ(result, "Test");
    free(result);
}

// Тест для парсинга простих вузлів
TEST_F(BejParserTest, ParseSimpleIntegerNode) {
    unsigned char data[] = {
        0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04
    };
    
    struct bej_node* node = (struct bej_node*)calloc(1, sizeof(struct bej_node));
    unsigned char* ptr = data;
    unsigned char* end = data + sizeof(data);
    
    parse_sflv_recursion(node, &ptr, end, nullptr, data);
    
    EXPECT_EQ(node->sequence, 1);
    EXPECT_EQ(node->format, 3);
    EXPECT_EQ(node->length, 4);
    ASSERT_TRUE(node->value != nullptr);
    EXPECT_EQ(*(int*)node->value, 0x01020304);
    
    free_bej_node(node);
}

TEST_F(BejParserTest, ParseBooleanNode) {
    unsigned char data[] = {0x04, 0x06, 0x01, 0x01};
    
    struct bej_node* node = (struct bej_node*)calloc(1, sizeof(struct bej_node));
    unsigned char* ptr = data;
    unsigned char* end = data + sizeof(data);
    
    parse_sflv_recursion(node, &ptr, end, nullptr, data);
    
    EXPECT_EQ(node->sequence, 2);
    EXPECT_EQ(node->format, 6);
    EXPECT_TRUE(node->value != nullptr);
    EXPECT_EQ(*(int*)node->value, 1);
    
    free_bej_node(node);
}

// Тест для map функцій
TEST_F(BejParserTest, LoadMapBasic) {
    FILE* f = fopen("test_map.map", "w");
    ASSERT_TRUE(f != nullptr);
    fprintf(f, "1: CapacityMiB\n2: DataWidthBits\n3: AllowedSpeedsMHz\n");
    fclose(f);
    
    size_t count = 0;
    struct field_map* map = load_map("test_map.map", &count);
    
    ASSERT_TRUE(map != nullptr);
    EXPECT_EQ(count, 3);
    EXPECT_EQ(map[0].sequence, 1);
    EXPECT_STREQ(map[0].name, "CapacityMiB");
    EXPECT_EQ(map[1].sequence, 2);
    EXPECT_STREQ(map[1].name, "DataWidthBits");
    
    free_map(map, count);
    remove("test_map.map");
}

TEST_F(BejParserTest, GetFieldNameFound) {
    struct field_map map[] = {
        {1, strdup("CapacityMiB")},
        {2, strdup("DataWidthBits")},
        {3, strdup("AllowedSpeedsMHz")}
    };
    
    const char* name = get_field_name(2, map, 3);
    EXPECT_STREQ(name, "DataWidthBits");
    
    for (size_t i = 0; i < 3; i++) {
        free(map[i].name);
    }
}

TEST_F(BejParserTest, GetFieldNameNotFound) {
    struct field_map map[] = {
        {1, strdup("CapacityMiB")},
        {3, strdup("AllowedSpeedsMHz")}
    };
    
    const char* name = get_field_name(2, map, 2);
    EXPECT_EQ(name, nullptr);

    for (size_t i = 0; i < 2; i++) {
        free(map[i].name);
    }
}

// Тест для пам'яті
TEST_F(BejParserTest, MemoryCleanup) {

    struct bej_node* parent = (struct bej_node*)calloc(1, sizeof(struct bej_node));
    parent->children_count = 2;
    parent->children = (struct bej_node**)calloc(2, sizeof(struct bej_node*));
    
    parent->children[0] = (struct bej_node*)calloc(1, sizeof(struct bej_node));
    parent->children[0]->value = malloc(sizeof(int));
    *(int*)parent->children[0]->value = 42;
    
    parent->children[1] = (struct bej_node*)calloc(1, sizeof(struct bej_node));
    parent->children[1]->value = strdup("test");
    
    EXPECT_NO_FATAL_FAILURE(free_bej_node(parent));
}

// Boundary tests
TEST_F(BejParserTest, EmptyData) {
    unsigned char data[] = {};
    struct bej_node* result = parse_sflv_init(data, 0, nullptr);
    EXPECT_EQ(result, nullptr);
}

TEST_F(BejParserTest, MalformedVarint) {
    unsigned char data[] = {0x80, 0x80};
    unsigned char* ptr = data;
    unsigned char* end = data + sizeof(data);
    
    uint64_t result = read_varint_u64(&ptr, end);
    EXPECT_TRUE(result == 0 || ptr == end);
}