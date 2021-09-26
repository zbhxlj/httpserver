#include <gtest/gtest.h>
#include "../include/config.h"

namespace {
    TEST(ConfigParserTest, plain){
        EXPECT_EQ(
            web::WebConfig("127.0.0.1", 8080, "/home/zbh/project/csWeb"),  
            web::LoadConfig("/home/zbh/project/csWeb/config/config.yaml")
        );
    }
}