#include <gtest/gtest.h>
#include "../src/config.h"

namespace {
    TEST(ConfigParserTest, plain){
        EXPECT_EQ(
            webserver::WebConfig("127.0.0.1", 8080, "/home/zbh/project/csWeb"),  
            webserver::load_config("/home/zbh/project/csWeb/config/config.yaml")
        );
    }
}