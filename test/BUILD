package(default_visibility = ["//visibility:public"])

cc_library(
    name = "mocks",
    hdrs = ["mocks.h"],
)

cc_test(
    name = "queue_test",
    srcs = ["queue_test.cc"],
    deps = [
        ":mocks",
        "//:lib",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)