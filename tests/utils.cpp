#include <gtest/gtest.h>

#include "../va-recorder/src/source/source.h"
#include "../va-recorder/src/utils.h"

class UtilsTest : public ::testing::Test {

public:
    UtilsTest() {
    }

    ~UtilsTest() {
    }

    void SetUp() {
    }

    void TearDown() {
    }
};

TEST_F(UtilsTest, MakeEndpoint1) {
    va::Source source{"camera-1", "rtsp://192.168.0.1:554/stream"};
    auto endpoint = va::utils::make_endpoint(source);
    ASSERT_EQ(endpoint, "rtsp://192.168.0.1:554/stream");
}
TEST_F(UtilsTest, MakeEndpoint2) {
    va::Source source{"camera-1", "rtsp://192.168.0.1:554/stream", "admin", "admin"};
    auto endpoint = va::utils::make_endpoint(source);
    ASSERT_EQ(endpoint, "rtsp://admin:admin@192.168.0.1:554/stream");
}

TEST_F(UtilsTest, MakeEndpoint3) {
    va::Source source{"camera-1", "192.168.0.1:554/stream", "admin", "admin"};
    try {
        auto endpoint = va::utils::make_endpoint(source);
        FAIL();
    } catch (std::exception &ex) {
        EXPECT_STREQ("could not make endpoint for video "
                     "source(camera-1), incorrect url address",
                     ex.what());
    }
}
TEST_F(UtilsTest, MakeEndpoint4) {
    va::Source source{"camera-1", "/dev/video0"};
    auto endpoint = va::utils::make_endpoint(source);
    ASSERT_EQ(endpoint, "/dev/video0");
}
TEST_F(UtilsTest, MakeEndpoint5) {
    va::Source source{"camera-1", "192.168.0.1:554/stream", "admin"};
    try {
        auto endpoint = va::utils::make_endpoint(source);
        FAIL();
    } catch (std::exception &ex) {
        EXPECT_STREQ("'username' or 'password' are empty. Both "
                     "must be specified if one is specified",
                     ex.what());
    }
}

TEST_F(UtilsTest, ForamtDateTimeToPath1) {
    time_t secs = 1712932044;
    std::string filename = "2024/04/12/14";
    ASSERT_EQ(filename, va::utils::format_datetime_to_path(secs));
}
TEST_F(UtilsTest, ForamtDateTimeToPath2) {
    time_t secs = 1711934844;
    std::string filename = "2024/04/01/01";
    ASSERT_EQ(filename, va::utils::format_datetime_to_path(secs));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}