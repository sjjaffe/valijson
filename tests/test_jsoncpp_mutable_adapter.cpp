
#include <gtest/gtest.h>

#include <valijson/adapters/jsoncpp_mutable_adapter.hpp>
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>


class TestJsonCppMutableAdapter : public testing::Test
{

};

TEST_F(TestJsonCppMutableAdapter, BasicArrayIteration)
{
    const unsigned int numElements = 10;

    // Create a jsoncpp document that consists of an array of numbers
    Json::Value document(Json::arrayValue);
    for (unsigned int i = 0; i < numElements; i++) {
        document.append(Json::Value(i));
    }

    // Ensure that wrapping the document preserves the array and does not allow
    // it to be cast to other types
    valijson::adapters::mut::MutableJsonCppAdapter adapter(document);
#if VALIJSON_USE_EXCEPTIONS
    ASSERT_NO_THROW( adapter.getArray() );
    ASSERT_ANY_THROW( adapter.getBool() );
    ASSERT_ANY_THROW( adapter.getDouble() );
    ASSERT_ANY_THROW( adapter.getObject() );
    ASSERT_ANY_THROW( adapter.getString() );
#endif
    // Ensure that the array contains the expected number of elements
    EXPECT_EQ( numElements, adapter.getArray().size() );

    // Ensure that the elements are returned in the order they were inserted
    unsigned int expectedValue = 0;
    for (const valijson::adapters::mut::MutableJsonCppAdapter value : adapter.getArray()) {
        ASSERT_TRUE( value.isNumber() );
        EXPECT_EQ( double(expectedValue), value.getNumber() );
        expectedValue++;
    }

    // Ensure that the correct number of elements were iterated over
    EXPECT_EQ(numElements, expectedValue);
}

TEST_F(TestJsonCppMutableAdapter, BasicObjectIteration)
{
    const unsigned int numElements = 10;

    // Create a jsoncpp document that consists of an object that maps numeric
    // strings their corresponding numeric values
    Json::Value document(Json::objectValue);
    for (unsigned int i = 0; i < numElements; i++) {
        std::string name(std::to_string(i));
        document[name] = Json::Value(double(i));
    }

    // Ensure that wrapping the document preserves the object and does not
    // allow it to be cast to other types
    valijson::adapters::mut::MutableJsonCppAdapter adapter(document);
#if VALIJSON_USE_EXCEPTIONS
    ASSERT_NO_THROW( adapter.getObject() );
    ASSERT_ANY_THROW( adapter.getArray() );
    ASSERT_ANY_THROW( adapter.getBool() );
    ASSERT_ANY_THROW( adapter.getDouble() );
    ASSERT_ANY_THROW( adapter.getString() );
#endif

    // Ensure that the object contains the expected number of members
    EXPECT_EQ( numElements, adapter.getObject().size() );

    // Ensure that the members are returned in the order they were inserted
    unsigned int expectedValue = 0;
    for (const valijson::adapters::mut::MutableJsonCppAdapter::ObjectMember member : adapter.getObject()) {
        ASSERT_TRUE( member.second.isNumber() );
        EXPECT_EQ( std::to_string(expectedValue), member.first );
        EXPECT_EQ( double(expectedValue), member.second.getDouble() );
        expectedValue++;
    }

    // Ensure that the correct number of elements were iterated over
    EXPECT_EQ( numElements, expectedValue );
}

TEST_F(TestJsonCppMutableAdapter, CanGenerateDefaultSchema) {
    valijson::Schema schema;
    {
        Json::Value schemaDocument(Json::objectValue);
        schemaDocument["properties"]["A"]["default"] = 5;
        valijson::SchemaParser parser;
        valijson::adapters::mut::MutableJsonCppAdapter adapter(schemaDocument);
        parser.populateSchema(adapter, schema);
    }

    Json::Value document(Json::objectValue);

    valijson::Validator validator;
    valijson::adapters::mut::MutableJsonCppAdapter adapter(document);
    validator.validate(schema, adapter, nullptr);

    EXPECT_EQ(document.size(), 1);
    EXPECT_EQ(document["A"].asInt(), 5) << document;
}

TEST_F(TestJsonCppMutableAdapter, CanGenerateDefaultSchemaWithImmutable) {
    valijson::Schema schema;
    {
        Json::Value schemaDocument(Json::objectValue);
        schemaDocument["properties"]["A"]["default"] = 5;
        valijson::SchemaParser parser;
        valijson::adapters::mut::JsonCppAdapter adapter(schemaDocument);
        parser.populateSchema(adapter, schema);
    }

    Json::Value document(Json::objectValue);

    valijson::Validator validator;
    valijson::adapters::mut::MutableJsonCppAdapter adapter(document);
    validator.validate(schema, adapter, nullptr);

    EXPECT_EQ(document.size(), 1);
    EXPECT_EQ(document["A"].asInt(), 5) << document;
}

TEST_F(TestJsonCppMutableAdapter, ImmutableVersionWillNotChange) {
    valijson::Schema schema;
    {
        Json::Value schemaDocument(Json::objectValue);
        schemaDocument["properties"]["A"]["default"] = 5;
        valijson::SchemaParser parser;
        valijson::adapters::mut::JsonCppAdapter adapter(schemaDocument);
        parser.populateSchema(adapter, schema);
    }

    Json::Value document(Json::objectValue);

    valijson::Validator validator;
    valijson::adapters::mut::JsonCppAdapter adapter(document);
    validator.validate(schema, adapter, nullptr);

    EXPECT_EQ(document.size(), 0);
}

TEST_F(TestJsonCppMutableAdapter, CanGenerateDefaultSchemaWithEntirelyDifferentType) {
    valijson::Schema schema;
    {
        nlohmann::json schemaDocument;
        schemaDocument["properties"]["A"]["default"] = 5;
        valijson::SchemaParser parser;
        valijson::adapters::NlohmannJsonAdapter adapter(schemaDocument);
        parser.populateSchema(adapter, schema);
    }

    Json::Value document(Json::objectValue);

    valijson::Validator validator;
    valijson::adapters::mut::MutableJsonCppAdapter adapter(document);
    validator.validate(schema, adapter, nullptr);

    EXPECT_EQ(document.size(), 1);
    EXPECT_EQ(document["A"].asInt(), 5) << document;
}
