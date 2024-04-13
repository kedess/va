#include <gtest/gtest.h>

#include "../va-recorder/src/source/source.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "../rapidjson/document.h"
#include "../rapidjson/filereadstream.h"
#include "../rapidjson/rapidjson.h"
#include "../rapidjson/reader.h"
#pragma GCC diagnostic pop

class SourceTest : public ::testing::Test {

public:
    SourceTest() {
    }

    ~SourceTest() {
    }

    void SetUp() {
    }

    void TearDown() {
    }
};

TEST_F(SourceTest, ParseSourceFromJSON1) {
    auto text = R"(
        {
            "id":"camera-1",
            "url":"rtsp://192.168.0.1:554/stream"
        }
    )";
    rapidjson::Document doc;
    doc.Parse(text);
    auto source = va::Source::from_json(doc);
    ASSERT_EQ(source.id(), "camera-1");
    ASSERT_EQ(source.url(), "rtsp://192.168.0.1:554/stream");
    ASSERT_EQ(source.username(), std::nullopt);
    ASSERT_EQ(source.password(), std::nullopt);
}
TEST_F(SourceTest, ParseSourceFromJSON2) {
    auto text = R"(
        {
            "id":"camera-1",
            "url":"rtsp://192.168.0.1:554/stream",
            "username":"admin",
            "password":"admin"
        }
    )";
    rapidjson::Document doc;
    doc.Parse(text);
    auto source = va::Source::from_json(doc);
    ASSERT_EQ(source.id(), "camera-1");
    ASSERT_EQ(source.url(), "rtsp://192.168.0.1:554/stream");
    ASSERT_EQ(source.username().value(), "admin");
    ASSERT_EQ(source.password().value(), "admin");
}
TEST_F(SourceTest, ParseSourceFromJSON3) {
    auto text = R"(
        {
            "url":"rtsp://192.168.0.1:554/stream",
            "username":"admin",
            "password":"admin"
        }
    )";
    rapidjson::Document doc;
    doc.Parse(text);
    try {
        auto source = va::Source::from_json(doc);
        FAIL();
    } catch (std::exception &ex) {
        EXPECT_STREQ("source parsing error, not found fields 'id' or 'url'", ex.what());
    }
}
TEST_F(SourceTest, ParseSourceFromJSON4) {
    auto text = R"(
        {
            "id":"camera-1",
            "username":"admin",
            "password":"admin"
        }
    )";
    rapidjson::Document doc;
    doc.Parse(text);
    try {
        auto source = va::Source::from_json(doc);
        FAIL();
    } catch (std::exception &ex) {
        EXPECT_STREQ("source parsing error, not found fields 'id' or 'url'", ex.what());
    }
}
TEST_F(SourceTest, ParseSourceFromJSON5) {
    auto text = R"(
        {
            "id":"",
            "url":"rtsp://192.168.0.1:554/stream",
            "username":"admin",
            "password":"admin"
        }
    )";
    rapidjson::Document doc;
    doc.Parse(text);
    try {
        auto source = va::Source::from_json(doc);
        FAIL();
    } catch (std::exception &ex) {
        EXPECT_STREQ("source parsing error, empty fields 'id' or 'url'", ex.what());
    }
}
TEST_F(SourceTest, ParseSourceFromJSON6) {
    auto text = R"(
        {
            "id":"camera-1",
            "url":"",
            "username":"admin",
            "password":"admin"
        }
    )";
    rapidjson::Document doc;
    doc.Parse(text);
    try {
        auto source = va::Source::from_json(doc);
        FAIL();
    } catch (std::exception &ex) {
        EXPECT_STREQ("source parsing error, empty fields 'id' or 'url'", ex.what());
    }
}
TEST_F(SourceTest, ParseSourceFromJSON7) {
    auto text = R"(
        {
            "id":"camera-1",
            "url":"rtsp://192.168.0.1:554/stream",
            "username":"",
            "password":"admin"
        }
    )";
    rapidjson::Document doc;
    doc.Parse(text);
    try {
        auto source = va::Source::from_json(doc);
        FAIL();
    } catch (std::exception &ex) {
        EXPECT_STREQ("source parsing error, empty fields 'username' or 'password'", ex.what());
    }
}
TEST_F(SourceTest, ParseSourceFromJSON8) {
    auto text = R"(
        {
            "id":"camera-1",
            "url":"rtsp://192.168.0.1:554/stream",
            "username":"admin",
            "password":""
        }
    )";
    rapidjson::Document doc;
    doc.Parse(text);
    try {
        auto source = va::Source::from_json(doc);
        FAIL();
    } catch (std::exception &ex) {
        EXPECT_STREQ("source parsing error, empty fields 'username' or 'password'", ex.what());
    }
}
TEST_F(SourceTest, ParseSourceFromJSON9) {
    auto text = R"(
        {
            "id",
            "url":"rtsp://192.168.0.1:554/stream",
            "username":"admin",
            "password":"admin"
        }
    )";
    rapidjson::Document doc;
    doc.Parse(text);
    try {
        auto source = va::Source::from_json(doc);
        FAIL();
    } catch (std::exception &ex) {
        EXPECT_STREQ("source parsing error, document is not an object", ex.what());
    }
}
TEST_F(SourceTest, ParseSourceFromJSON10) {
    auto text = R"(
        {
            "id":"camera-1",
            "url":"rtsp://192.168.0.1:554/stream",
            "fake_field": 123
        }
    )";
    rapidjson::Document doc;
    doc.Parse(text);
    auto source = va::Source::from_json(doc);
    ASSERT_EQ(source.id(), "camera-1");
    ASSERT_EQ(source.url(), "rtsp://192.168.0.1:554/stream");
    ASSERT_EQ(source.username(), std::nullopt);
    ASSERT_EQ(source.password(), std::nullopt);
}
TEST_F(SourceTest, ParseSourceFromJSON11) {
    auto text = R"(
        {
            "id":1234,
            "url":"rtsp://192.168.0.1:554/stream"
        }
    )";
    rapidjson::Document doc;
    doc.Parse(text);
    try {
        auto source = va::Source::from_json(doc);
        FAIL();
    } catch (std::exception &ex) {
        EXPECT_STREQ("source parsing error, fields 'id' or 'url' must be string", ex.what());
    }
}
TEST_F(SourceTest, ParseSourceFromJSON12) {
    auto text = R"(
        {
            "id":"camera-1",
            "url":true
        }
    )";
    rapidjson::Document doc;
    doc.Parse(text);
    try {
        auto source = va::Source::from_json(doc);
        FAIL();
    } catch (std::exception &ex) {
        EXPECT_STREQ("source parsing error, fields 'id' or 'url' must be string", ex.what());
    }
}
TEST_F(SourceTest, ParseSourceFromJSON13) {
    auto text = R"(
        {
            "id":"camera-1",
            "url":"rtsp://192.168.0.1:554/stream",
            "username":123,
            "password":"admin"
        }
    )";
    rapidjson::Document doc;
    doc.Parse(text);
    try {
        auto source = va::Source::from_json(doc);
        FAIL();
    } catch (std::exception &ex) {
        EXPECT_STREQ("source parsing error, fields 'username' or 'password' "
                     "must be string",
                     ex.what());
    }
}
TEST_F(SourceTest, ParseSourceFromJSON14) {
    auto text = R"(
        {
            "id":"camera-1",
            "url":"rtsp://192.168.0.1:554/stream",
            "username":"admin",
            "password":false
        }
    )";
    rapidjson::Document doc;
    doc.Parse(text);
    try {
        auto source = va::Source::from_json(doc);
        FAIL();
    } catch (std::exception &ex) {
        EXPECT_STREQ("source parsing error, fields 'username' or 'password' "
                     "must be string",
                     ex.what());
    }
}
TEST_F(SourceTest, ParseSourceFromJSON15) {
    auto text = R"(
        {
            "id": "camera-1",
            "url":"rtsp://192.168.0.1:554/stream",
            "username":"admin",
            "password":"admin",
        }
    )";
    rapidjson::Document doc;
    doc.Parse(text);
    try {
        auto source = va::Source::from_json(doc);
        FAIL();
    } catch (std::exception &ex) {
        EXPECT_STREQ("source parsing error, document is not an object", ex.what());
    }
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}