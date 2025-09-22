#include <gtest/gtest.h>
#include "bej_wrapper.h"

class JsonWriterTest : public ::testing::Test {
protected:
    void SetUp() override {
        json_str = dynamic_string_init();
    }

    void TearDown() override {
        if (json_str) {
            free(json_str->data);
            free(json_str);
        }
    }

    struct dynamic_string* json_str;
};

TEST_F(JsonWriterTest, DynamicStringInit) {
    ASSERT_TRUE(json_str != nullptr);
    EXPECT_EQ(json_str->length, 0);
    EXPECT_GE(json_str->capacity, 1024);
    EXPECT_STREQ(json_str->data, "");
}

TEST_F(JsonWriterTest, DynamicStringAppend) {
    dynamic_string_append(json_str, "Hello");
    EXPECT_STREQ(json_str->data, "Hello");
    EXPECT_EQ(json_str->length, 5);
    
    dynamic_string_append(json_str, " World");
    EXPECT_STREQ(json_str->data, "Hello World");
    EXPECT_EQ(json_str->length, 11);
}

TEST_F(JsonWriterTest, DynamicStringRealloc) {
    // Тест автоматичне перевиділення пам'яті
    std::string long_string(2000, 'A');
    dynamic_string_append(json_str, long_string.c_str());
    
    EXPECT_EQ(json_str->length, 2000);
    EXPECT_GE(json_str->capacity, 2000);
}

TEST_F(JsonWriterTest, FormatInteger) {
    struct bej_node node = {};
    node.format = 3; // INTEGER
    node.length = 4;
    int value = 42;
    node.value = &value;
    
    parse_bej_node_to_str_recursion(&node, json_str, "test_int", 0, nullptr, 0);
    EXPECT_STREQ(json_str->data, "\"test_int\": 42");
}

TEST_F(JsonWriterTest, FormatString) {
    struct bej_node node = {};
    node.format = 5; // STRING
    node.value = strdup("hello");
    
    parse_bej_node_to_str_recursion(&node, json_str, "test_str", 0, nullptr, 0);
    EXPECT_STREQ(json_str->data, "\"test_str\": \"hello\"");
    
    free(node.value);
}

TEST_F(JsonWriterTest, FormatBooleanTrue) {
    struct bej_node node = {};
    node.format = 6; // BOOLEAN
    int value = 1;
    node.value = &value;
    
    parse_bej_node_to_str_recursion(&node, json_str, "test_bool", 0, nullptr, 0);
    EXPECT_STREQ(json_str->data, "\"test_bool\": true");
}

TEST_F(JsonWriterTest, FormatBooleanFalse) {
    struct bej_node node = {};
    node.format = 6; // BOOLEAN
    int value = 0;
    node.value = &value;
    
    parse_bej_node_to_str_recursion(&node, json_str, "test_bool", 0, nullptr, 0);
    EXPECT_STREQ(json_str->data, "\"test_bool\": false");
}

TEST_F(JsonWriterTest, FormatNull) {
    struct bej_node node = {};
    node.format = 0; // NULL
    
    parse_bej_node_to_str_recursion(&node, json_str, "test_null", 0, nullptr, 0);
    EXPECT_STREQ(json_str->data, "\"test_null\": null");
}

TEST_F(JsonWriterTest, FormatSetWithMap) {
    struct bej_node* child1 = (struct bej_node*)calloc(1, sizeof(struct bej_node));
    child1->format = 3; // INTEGER
    child1->sequence = 1;
    int value1 = 100;
    child1->value = &value1;
    
    struct bej_node* child2 = (struct bej_node*)calloc(1, sizeof(struct bej_node));
    child2->format = 5; // STRING
    child2->sequence = 2;
    child2->value = strdup("test");
    
    struct bej_node parent = {};
    parent.format = 1; // SET
    parent.children_count = 2;
    parent.children = (struct bej_node**)calloc(2, sizeof(struct bej_node*));
    parent.children[0] = child1;
    parent.children[1] = child2;
    
    struct field_map map[] = {
        {1, strdup("Capacity")},
        {2, strdup("Name")}
    };
    
    parse_bej_node_to_str_recursion(&parent, json_str, nullptr, 0, map, 2);
    
    // Перевірка що JSON містить очікувані поля
    EXPECT_TRUE(strstr(json_str->data, "\"Capacity\": 100") != nullptr);
    EXPECT_TRUE(strstr(json_str->data, "\"Name\": \"test\"") != nullptr);
    
    // Cleanup
    free(parent.children);
    free(child2->value);
    free(child1);
    free(child2);
    free_map(map, 2);
}

TEST_F(JsonWriterTest, UnknownFormat) {
    struct bej_node node = {};
    node.format = 99; // Невідомий формат
    
    parse_bej_node_to_str_recursion(&node, json_str, "unknown", 0, nullptr, 0);
    EXPECT_STREQ(json_str->data, "\"unknown\": \"<unknown>\"");
}