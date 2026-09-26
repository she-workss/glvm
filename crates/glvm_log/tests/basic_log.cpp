#include "glvm_log/level.hpp"
#include "glvm_log/logger.hpp"

#include <gtest/gtest.h>

using namespace glvm_log;

TEST(LogLevelTest, LevelNameReturnsCorrectStrings) {
    EXPECT_EQ(level_name(LogLevel::Trace), "TRACE");
    EXPECT_EQ(level_name(LogLevel::Debug), "DEBUG");
    EXPECT_EQ(level_name(LogLevel::Info), "INFO ");
    EXPECT_EQ(level_name(LogLevel::Warn), "WARN ");
    EXPECT_EQ(level_name(LogLevel::Error), "ERROR");
    EXPECT_EQ(level_name(LogLevel::Off), "OFF  ");
}

TEST(LogLevelTest, LevelFromStrKnownNames) {
    EXPECT_EQ(level_from_str("trace"), LogLevel::Trace);
    EXPECT_EQ(level_from_str("debug"), LogLevel::Debug);
    EXPECT_EQ(level_from_str("info"), LogLevel::Info);
    EXPECT_EQ(level_from_str("warn"), LogLevel::Warn);
    EXPECT_EQ(level_from_str("error"), LogLevel::Error);
    EXPECT_EQ(level_from_str("off"), LogLevel::Off);
}

TEST(LogLevelTest, LevelFromStrUnknownDefaultsToInfo) {
    EXPECT_EQ(level_from_str(""), LogLevel::Info);
    EXPECT_EQ(level_from_str("UNKNOWN"), LogLevel::Info);
    EXPECT_EQ(level_from_str("INFO"), LogLevel::Info);
}

TEST(LogLevelTest, LevelOrdering) {
    EXPECT_LT(
        static_cast<i32>(LogLevel::Trace),
        static_cast<i32>(LogLevel::Debug)
    );
    EXPECT_LT(
        static_cast<i32>(LogLevel::Debug),
        static_cast<i32>(LogLevel::Info)
    );
    EXPECT_LT(
        static_cast<i32>(LogLevel::Info),
        static_cast<i32>(LogLevel::Warn)
    );
    EXPECT_LT(
        static_cast<i32>(LogLevel::Warn),
        static_cast<i32>(LogLevel::Error)
    );
    EXPECT_LT(
        static_cast<i32>(LogLevel::Error),
        static_cast<i32>(LogLevel::Off)
    );
}

struct LoggerTest: ::testing::Test {
    Logger logger;
};

TEST_F(LoggerTest, DefaultLevelIsInfo) {
    EXPECT_EQ(logger.global_level, LogLevel::Info);
    EXPECT_TRUE(logger.filters.empty());
}

TEST_F(LoggerTest, ParseFilterBareLevel) {
    logger.parse_filter("debug");
    EXPECT_EQ(logger.global_level, LogLevel::Debug);
    EXPECT_TRUE(logger.filters.empty());
}

TEST_F(LoggerTest, ParseFilterSingleCategory) {
    logger.parse_filter("glvm_render=warn");
    EXPECT_EQ(logger.global_level, LogLevel::Info);
    EXPECT_EQ(logger.filters.at("glvm_render"), LogLevel::Warn);
}

TEST_F(LoggerTest, ParseFilterMultipleCategories) {
    logger.parse_filter("glvm_render=error,glvm_ecs=trace");
    EXPECT_EQ(logger.filters.at("glvm_render"), LogLevel::Error);
    EXPECT_EQ(logger.filters.at("glvm_ecs"), LogLevel::Trace);
    EXPECT_EQ(logger.global_level, LogLevel::Info);
}

TEST_F(LoggerTest, ParseFilterMixed) {
    logger.parse_filter("glvm_render=error,debug");
    EXPECT_EQ(logger.global_level, LogLevel::Debug);
    EXPECT_EQ(logger.filters.at("glvm_render"), LogLevel::Error);
}

TEST_F(LoggerTest, ParseFilterEmptyString) {
    logger.parse_filter("");
    EXPECT_EQ(logger.global_level, LogLevel::Info);
    EXPECT_TRUE(logger.filters.empty());
}

TEST_F(LoggerTest, ParseFilterTrailingComma) {
    logger.parse_filter("glvm_render=warn,");
    EXPECT_EQ(logger.filters.at("glvm_render"), LogLevel::Warn);
    EXPECT_EQ(logger.global_level, LogLevel::Info);
}

TEST_F(LoggerTest, ShouldLogRespectsGlobalLevel) {
    logger.global_level = LogLevel::Warn;
    EXPECT_FALSE(logger.should_log("any", LogLevel::Trace));
    EXPECT_FALSE(logger.should_log("any", LogLevel::Debug));
    EXPECT_FALSE(logger.should_log("any", LogLevel::Info));
    EXPECT_TRUE(logger.should_log("any", LogLevel::Warn));
    EXPECT_TRUE(logger.should_log("any", LogLevel::Error));
}

TEST_F(LoggerTest, ShouldLogCategoryOverridesGlobal) {
    logger.global_level = LogLevel::Info;
    logger.filters["glvm_ecs"] = LogLevel::Trace;
    EXPECT_TRUE(logger.should_log("glvm_ecs", LogLevel::Trace));
    EXPECT_FALSE(logger.should_log("glvm_render", LogLevel::Debug));
    EXPECT_TRUE(logger.should_log("glvm_render", LogLevel::Info));
}

TEST_F(LoggerTest, OffLevelSilencesEverything) {
    logger.global_level = LogLevel::Off;
    EXPECT_FALSE(logger.should_log("any", LogLevel::Error));
    EXPECT_FALSE(logger.should_log("any", LogLevel::Warn));
}

TEST_F(LoggerTest, CategoryOffSilencesThatCategory) {
    logger.global_level = LogLevel::Debug;
    logger.filters["glvm_mesh"] = LogLevel::Off;
    EXPECT_FALSE(logger.should_log("glvm_mesh", LogLevel::Error));
    EXPECT_TRUE(logger.should_log("glvm_render", LogLevel::Debug));
}

TEST_F(LoggerTest, EffectiveLevelNoOverride) {
    logger.global_level = LogLevel::Warn;
    EXPECT_EQ(logger.effective_level("glvm_ecs"), LogLevel::Warn);
}

TEST_F(LoggerTest, EffectiveLevelWithOverride) {
    logger.global_level = LogLevel::Warn;
    logger.filters["glvm_ecs"] = LogLevel::Trace;
    EXPECT_EQ(logger.effective_level("glvm_ecs"), LogLevel::Trace);
    EXPECT_EQ(logger.effective_level("glvm_render"), LogLevel::Warn);
}

TEST(LoggerSingletonTest, ConfigureAppliesLevelAndFilter) {
    Logger::configure(LogLevel::Info, "");
    Logger::configure(LogLevel::Debug, "glvm_render=error");
    auto& inst = Logger::instance();
    EXPECT_EQ(inst.global_level, LogLevel::Debug);
    EXPECT_EQ(inst.filters.at("glvm_render"), LogLevel::Error);
}

TEST(LoggerSingletonTest, ConfigureClearsPreviousFilters) {
    Logger::configure(LogLevel::Info, "glvm_ecs=trace");
    EXPECT_TRUE(Logger::instance().filters.count("glvm_ecs") > 0);
    Logger::configure(LogLevel::Info, "");
    EXPECT_TRUE(Logger::instance().filters.empty());
}
